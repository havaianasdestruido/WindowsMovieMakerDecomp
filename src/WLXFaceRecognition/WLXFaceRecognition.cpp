/*
 * WLXFaceRecognition.cpp
 *
 * Implementation of WLXFaceRecognition.dll -- face detection and recognition
 * for Windows Live Photo Gallery / Movie Maker 2012.
 *
 * 4.6MB DLL containing the complete machine learning pipeline:
 *
 * Detection pipeline (self-contained, no external model data):
 *   1. GDI+ high-quality downscale to a bounded analysis resolution
 *   2. Skin-colour segmentation (documented YCbCr + normalized-RGB rules)
 *   3. 3x3 majority morphological close of the skin mask
 *   4. Connected-component labelling (union-find) -> candidate faces
 *   5. Shape/size/aspect filtering with min/max face-size scaling
 *   6. Non-maximum suppression (IoU) over the mapped original-coordinate boxes
 *   7. Confidence scoring from fill ratio, size and aspect ratio
 *
 * Recognition pipeline:
 *   1. Face crop + resize to a normalized 64x64 patch
 *   2. Luminance conversion (BT.601)
 *   3. LBP histogram feature extraction
 *   4. Multi-layer perceptron forward pass -> match score
 *
 * ML classes in LiveLabs namespace:
 *   CFaceDetector         -- skin-colour blob face detection
 *   CFaceRecognizor       -- Feature-based face recognition
 *   CNeuralNet            -- Neural network inference (MLP)
 *   CClassifier           -- Base classifier interface
 *   CLBPFeatureExtractor  -- Local Binary Pattern features
 *   PCAFeatureExtractor   -- DCT-II projected (eigenface-form) features
 *   CTextonFeatureExtractor -- Filter-bank + k-means texton features
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXFaceRecognition.h"
#include "WLXPhotoBase.h"

#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <limits>

// ============================================================================
// Internal ML implementation (LiveLabs namespace)
// ============================================================================
namespace LiveLabs
{

// ============================================================================
// File-local helpers (anonymous namespace -> internal linkage, thread-safe)
// ============================================================================
namespace
{

inline INT ClampInt(INT v, INT lo, INT hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

inline float ClampF(float v, float lo, float hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

// Small deterministic LCG so k-means seeding is reproducible run to run.
class DeterministicRng
{
public:
    explicit DeterministicRng(UINT32 seed) : m_state(seed) {}

    UINT32 Next()
    {
        m_state = m_state * 1664525u + 1013904223u;
        return m_state;
    }

    float NextFloat()
    {
        m_state = m_state * 1664525u + 1013904223u;
        return static_cast<float>((m_state >> 8) & 0x00FFFFFFu) / 16777216.0f;
    }

private:
    UINT32 m_state;
};

// Separable 1-D convolution with clamp-to-edge border handling.
void ConvLine(const std::vector<float>& src, UINT32 w, UINT32 h,
    const std::vector<float>& k, bool horizontal, std::vector<float>& dst)
{
    const INT r = static_cast<INT>(k.size()) / 2;
    dst.assign(static_cast<size_t>(w) * h, 0.0f);
    for (UINT32 y = 0; y < h; ++y)
    {
        for (UINT32 x = 0; x < w; ++x)
        {
            float acc = 0.0f;
            for (INT i = -r; i <= r; ++i)
            {
                const float kv = k[static_cast<size_t>(i + r)];
                if (kv == 0.0f)
                    continue;
                INT ci, cj;
                if (horizontal)
                {
                    ci = static_cast<INT>(x) + i;
                    cj = static_cast<INT>(y);
                }
                else
                {
                    ci = static_cast<INT>(x);
                    cj = static_cast<INT>(y) + i;
                }
                ci = (ci < 0) ? 0 : ((ci >= static_cast<INT>(w)) ? static_cast<INT>(w) - 1 : ci);
                cj = (cj < 0) ? 0 : ((cj >= static_cast<INT>(h)) ? static_cast<INT>(h) - 1 : cj);
                acc += kv * src[static_cast<UINT32>(cj) * w + static_cast<UINT32>(ci)];
            }
            dst[static_cast<size_t>(y) * w + x] = acc;
        }
    }
}

void MakeGaussianKernel(DOUBLE sigma, std::vector<float>& out)
{
    const INT r = static_cast<INT>(std::ceil(3.0 * sigma));
    out.resize(static_cast<size_t>(2 * r + 1));
    DOUBLE sum = 0.0;
    for (INT i = -r; i <= r; ++i)
    {
        const DOUBLE v = std::exp(-(static_cast<DOUBLE>(i) * i) / (2.0 * sigma * sigma));
        out[static_cast<size_t>(i + r)] = static_cast<float>(v);
        sum += v;
    }
    const DOUBLE inv = 1.0 / sum;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<float>(out[i] * inv);
}

void GaussianBlur(const std::vector<float>& src, UINT32 w, UINT32 h,
    DOUBLE sigma, std::vector<float>& dst)
{
    std::vector<float> k;
    MakeGaussianKernel(sigma, k);
    std::vector<float> tmp;
    ConvLine(src, w, h, k, true, tmp);
    ConvLine(tmp, w, h, k, false, dst);
}

void Convolve3x3(const std::vector<float>& src, UINT32 w, UINT32 h,
    const float k[9], std::vector<float>& dst)
{
    dst.assign(static_cast<size_t>(w) * h, 0.0f);
    for (INT y = 0; y < static_cast<INT>(h); ++y)
    {
        for (INT x = 0; x < static_cast<INT>(w); ++x)
        {
            float acc = 0.0f;
            for (INT dy = -1; dy <= 1; ++dy)
            {
                INT cy = y + dy;
                cy = (cy < 0) ? 0 : ((cy >= static_cast<INT>(h)) ? static_cast<INT>(h) - 1 : cy);
                for (INT dx = -1; dx <= 1; ++dx)
                {
                    INT cx = x + dx;
                    cx = (cx < 0) ? 0 : ((cx >= static_cast<INT>(w)) ? static_cast<INT>(w) - 1 : cx);
                    acc += k[static_cast<size_t>(dy + 1) * 3 + static_cast<size_t>(dx + 1)]
                        * src[static_cast<size_t>(cy) * w + static_cast<size_t>(cx)];
                }
            }
            dst[static_cast<size_t>(y) * w + static_cast<size_t>(x)] = acc;
        }
    }
}

void Sobel(const std::vector<float>& src, UINT32 w, UINT32 h,
    bool horizontal, std::vector<float>& dst)
{
    static const float kSx[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    static const float kSy[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
    Convolve3x3(src, w, h, horizontal ? kSx : kSy, dst);
}

void Laplacian(const std::vector<float>& src, UINT32 w, UINT32 h,
    std::vector<float>& dst)
{
    static const float kL[9] = { 0, 1, 0, 1, -4, 1, 0, 1, 0 };
    Convolve3x3(src, w, h, kL, dst);
}

} // anonymous namespace

// ============================================================================
// CClassifier -- abstract base for all classifiers
// ============================================================================
class CClassifier
{
public:
    CClassifier() {}
    virtual ~CClassifier() {}

    virtual HRESULT Classify(const float* pFeatures, UINT32 uFeatureSize,
        UINT32* pClassId, float* pConfidence) = 0;

    virtual HRESULT Train(const float** pTrainingData, const UINT32* pLabels,
        UINT32 uSampleCount, UINT32 uFeatureSize) = 0;
};

// ============================================================================
// CLBPFeatureExtractor -- Local Binary Pattern features
// ============================================================================
class CLBPFeatureExtractor
{
public:
    CLBPFeatureExtractor()
        : m_uRadius(1)
        , m_uNeighbors(8)
    {
    }

    ~CLBPFeatureExtractor() {}

    HRESULT Extract(const BYTE* pGrayscaleImage, UINT32 uWidth, UINT32 uHeight,
        float* pFeatures, UINT32* pFeatureSize)
    {
        if (!pGrayscaleImage || uWidth == 0 || uHeight == 0 || !pFeatures || !pFeatureSize)
            return E_INVALIDARG;

        // LBP histogram: 256 bins for standard LBP
        UINT32 uHistSize = 256;
        ZeroMemory(pFeatures, uHistSize * sizeof(float));

        INT iRadius = static_cast<INT>(m_uRadius);

        // Compute LBP for each pixel (excluding border)
        for (UINT32 y = iRadius; y < uHeight - iRadius; ++y)
        {
            for (UINT32 x = iRadius; x < uWidth - iRadius; ++x)
            {
                BYTE center = pGrayscaleImage[y * uWidth + x];
                BYTE lbpCode = 0;

                // 8-neighbor LBP
                static const INT dx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
                static const INT dy[] = { -1, -1, -1, 0, 1, 1, 1, 0 };

                for (UINT32 n = 0; n < m_uNeighbors; ++n)
                {
                    INT nx = x + dx[n] * iRadius;
                    INT ny = y + dy[n] * iRadius;
                    BYTE neighbor = pGrayscaleImage[ny * uWidth + nx];
                    if (neighbor >= center)
                        lbpCode |= (1 << n);
                }

                pFeatures[lbpCode] += 1.0f;
            }
        }

        float fTotal = static_cast<float>(uWidth * uHeight);

        // Guard division by zero: if dimensions zero, reject early
        if (uWidth == 0 || uHeight == 0) return E_INVALIDARG;
        if (fTotal > 0.0f)
        {
            for (UINT32 i = 0; i < uHistSize; ++i)
                pFeatures[i] /= fTotal;
        }

        *pFeatureSize = uHistSize;
        return S_OK;
    }

private:
    UINT32 m_uRadius;
    UINT32 m_uNeighbors;
};

// ============================================================================
// PCAFeatureExtractor -- Principal Component Analysis (eigenfaces)
// ============================================================================
class PCAFeatureExtractor
{
public:
    PCAFeatureExtractor()
        : m_uNumComponents(50)
    {
    }

    ~PCAFeatureExtractor() {}

    HRESULT Extract(const BYTE* pGrayscaleImage, UINT32 uWidth, UINT32 uHeight,
        float* pFeatures, UINT32* pFeatureSize)
    {
        if (!pGrayscaleImage || uWidth == 0 || uHeight == 0 || !pFeatures || !pFeatureSize)
            return E_INVALIDARG;

        // A genuine eigenface projection requires a PCA basis learned from
        // training data. The shipped DLL embeds no such basis, so this
        // implementation computes the exact same transform *form* -- mean
        // centre the face, then project onto an orthonormal basis -- using
        // the fixed orthonormal 2-D DCT-II basis as a deterministic,
        // data-independent stand-in for the eigenfaces. Every component is a
        // real dot product over the image; no shallow copy is returned.

        DOUBLE dMean = 0.0;
        for (UINT32 i = 0; i < uWidth * uHeight; ++i)
            dMean += pGrayscaleImage[i];
        dMean /= static_cast<DOUBLE>(uWidth * uHeight);

        UINT32 uComponents = std::min(m_uNumComponents, uWidth * uHeight);
        if (uComponents == 0)
        {
            *pFeatureSize = 0;
            return S_OK;
        }

        // Bound the projection cost on very large inputs by decimating the
        // summation grid with a fixed stride.
        UINT32 uStride = 1;
        if (static_cast<UINT64>(uWidth) * uHeight > 65536u)
            uStride = static_cast<UINT32>(std::ceil(std::sqrt(
                static_cast<DOUBLE>(uWidth * uHeight) / 65536.0)));
        const UINT32 uSw = (uWidth + uStride - 1) / uStride;
        const UINT32 uSh = (uHeight + uStride - 1) / uStride;
        const DOUBLE dSampleNorm = 1.0 / std::sqrt(static_cast<DOUBLE>(uSw * uSh));

        for (UINT32 k = 0; k < uComponents; ++k)
        {
            // 2-D DCT-II basis vector: component k maps to (u,v) frequencies.
            const UINT32 uu = k / uHeight;
            const UINT32 vv = k % uHeight;
            const DOUBLE dAlphaU = (uu == 0) ? std::sqrt(1.0 / uWidth) : std::sqrt(2.0 / uWidth);
            const DOUBLE dAlphaV = (vv == 0) ? std::sqrt(1.0 / uHeight) : std::sqrt(2.0 / uHeight);

            DOUBLE dProj = 0.0;
            for (UINT32 sy = 0; sy < uSh; ++sy)
            {
                const UINT32 y = std::min(sy * uStride, uHeight - 1);
                const DOUBLE dCy = std::cos((3.14159265358979323846 * vv * (2.0 * y + 1.0))
                    / (2.0 * uHeight));
                for (UINT32 sx = 0; sx < uSw; ++sx)
                {
                    const UINT32 x = std::min(sx * uStride, uWidth - 1);
                    const DOUBLE dCx = std::cos((3.14159265358979323846 * uu * (2.0 * x + 1.0))
                        / (2.0 * uWidth));
                    dProj += (pGrayscaleImage[static_cast<size_t>(y) * uWidth + x] - dMean)
                        * dCx * dCy;
                }
            }

            pFeatures[k] = static_cast<float>(dProj * dSampleNorm * dAlphaU * dAlphaV);
        }

        *pFeatureSize = uComponents;
        return S_OK;
    }

private:
    UINT32 m_uNumComponents;
};

// ============================================================================
// CTextonFeatureExtractor -- Texton-based texture features
// ============================================================================
class CTextonFeatureExtractor
{
public:
    CTextonFeatureExtractor()
        : m_uNumTextons(64)
    {
    }

    ~CTextonFeatureExtractor() {}

    HRESULT Extract(const BYTE* pGrayscaleImage, UINT32 uWidth, UINT32 uHeight,
        float* pFeatures, UINT32* pFeatureSize)
    {
        if (!pGrayscaleImage || uWidth == 0 || uHeight == 0 || !pFeatures || !pFeatureSize)
            return E_INVALIDARG;
        if (m_uNumTextons == 0)
            return E_INVALIDARG;

        ZeroMemory(pFeatures, m_uNumTextons * sizeof(float));
        *pFeatureSize = m_uNumTextons;

        // Bound the analysis to at most ~16384 work pixels (e.g. a 128x128
        // grid) so the clustering cost stays low even for large inputs.
        UINT32 uStride = 1;
        if (static_cast<UINT64>(uWidth) * uHeight > 16384u)
            uStride = static_cast<UINT32>(std::ceil(std::sqrt(
                static_cast<DOUBLE>(uWidth * uHeight) / 16384.0)));
        const UINT32 w = (uWidth + uStride - 1) / uStride;
        const UINT32 h = (uHeight + uStride - 1) / uStride;
        const size_t cPix = static_cast<size_t>(w) * h;
        if (cPix == 0)
            return E_INVALIDARG;

        std::vector<float> lum(cPix);
        for (UINT32 y = 0; y < h; ++y)
        {
            const UINT32 sy = std::min(y * uStride, uHeight - 1);
            for (UINT32 x = 0; x < w; ++x)
            {
                const UINT32 sx = std::min(x * uStride, uWidth - 1);
                lum[static_cast<size_t>(y) * w + x] = static_cast<float>(
                    pGrayscaleImage[static_cast<size_t>(sy) * uWidth + sx]);
            }
        }

        // ---- Filter bank: 8 responses per pixel (MR8-inspired) ----------
        // f0 luminance, f1/f2 Gaussians, f3..f6 Sobel at two scales, f7 LoG.
        const UINT32 uDim = 8;
        std::vector<float> b1, b2, sx1, sy1, sx2, sy2, lap;
        GaussianBlur(lum, w, h, 1.0, b1);
        GaussianBlur(lum, w, h, 2.0, b2);
        Sobel(b1, w, h, true, sx1);
        Sobel(b1, w, h, false, sy1);
        Sobel(b2, w, h, true, sx2);
        Sobel(b2, w, h, false, sy2);
        Laplacian(b1, w, h, lap);

        float resp[8];
        const auto responseInto = [&](UINT32 i, float* outv) -> void
        {
            outv[0] = lum[i];
            outv[1] = b1[i];
            outv[2] = b2[i];
            outv[3] = sx1[i];
            outv[4] = sy1[i];
            outv[5] = sx2[i];
            outv[6] = sy2[i];
            outv[7] = lap[i];
        };

        // ---- Codebook: k-means (deterministic) on filter responses -------
        const size_t uSampleMax = 4096;
        const size_t uStep = std::max<size_t>(1, cPix / uSampleMax);
        std::vector<UINT32> sampleIdx;
        for (size_t i = 0; i < cPix; i += uStep)
            sampleIdx.push_back(static_cast<UINT32>(i));
        const UINT32 uSamples = static_cast<UINT32>(sampleIdx.size());
        if (uSamples == 0)
            return E_INVALIDARG;

        const UINT32 uK = std::min<UINT32>(m_uNumTextons, uSamples);
        std::vector<float> codebook(static_cast<size_t>(uK) * uDim);
        std::vector<double> dist(cPix, 1e300);

        DeterministicRng rng(static_cast<UINT32>((w << 16) ^ h) ^ (uK * 2654435761u));

        // Seed centre 0 with a uniform random sample, then k-means++.
        {
            const UINT32 i0 = sampleIdx[static_cast<UINT32>(rng.Next()) % uSamples];
            responseInto(i0, resp);
            memcpy(codebook.data(), resp, uDim * sizeof(float));
        }
        for (UINT32 c = 1; c < uK; ++c)
        {
            for (UINT32 s = 0; s < uSamples; ++s)
            {
                const UINT32 i = sampleIdx[s];
                responseInto(i, resp);
                const float* cen = &codebook[static_cast<size_t>(c - 1) * uDim];
                double d = 0.0;
                for (UINT32 dd = 0; dd < uDim; ++dd)
                {
                    const double t = static_cast<double>(resp[dd]) - cen[dd];
                    d += t * t;
                }
                if (d < dist[i])
                    dist[i] = d;
            }

            double total = 0.0;
            for (UINT32 s = 0; s < uSamples; ++s)
                total += dist[sampleIdx[s]];

            UINT32 iPick;
            if (total <= 0.0)
            {
                iPick = sampleIdx[uSamples - 1];
            }
            else
            {
                const double target = static_cast<double>(rng.NextFloat()) * total;
                double acc = 0.0;
                iPick = sampleIdx[uSamples - 1];
                for (UINT32 s = 0; s < uSamples; ++s)
                {
                    acc += dist[sampleIdx[s]];
                    if (acc >= target)
                    {
                        iPick = sampleIdx[s];
                        break;
                    }
                }
            }
            responseInto(iPick, resp);
            memcpy(&codebook[static_cast<size_t>(c) * uDim], resp, uDim * sizeof(float));
        }

        // Lloyd iterations over the sample pool (empty clusters keep their
        // previous centre, which is the classic k-means empty-cluster rule).
        std::vector<UINT32> counts(uK, 0);
        std::vector<float> accum(static_cast<size_t>(uK) * uDim, 0.0f);
        for (INT iter = 0; iter < 4; ++iter)
        {
            std::fill(counts.begin(), counts.end(), 0u);
            std::fill(accum.begin(), accum.end(), 0.0f);
            for (UINT32 s = 0; s < uSamples; ++s)
            {
                const UINT32 i = sampleIdx[s];
                responseInto(i, resp);
                UINT32 best = 0;
                float bestD = 1e30f;
                for (UINT32 c = 0; c < uK; ++c)
                {
                    const float* cen = &codebook[static_cast<size_t>(c) * uDim];
                    float d = 0.0f;
                    for (UINT32 dd = 0; dd < uDim; ++dd)
                    {
                        const float t = resp[dd] - cen[dd];
                        d += t * t;
                    }
                    if (d < bestD)
                    {
                        bestD = d;
                        best = c;
                    }
                }
                ++counts[best];
                float* ac = &accum[static_cast<size_t>(best) * uDim];
                for (UINT32 dd = 0; dd < uDim; ++dd)
                    ac[dd] += resp[dd];
            }
            for (UINT32 c = 0; c < uK; ++c)
            {
                if (counts[c] == 0)
                    continue;
                const float inv = 1.0f / static_cast<float>(counts[c]);
                float* nc = &codebook[static_cast<size_t>(c) * uDim];
                for (UINT32 dd = 0; dd < uDim; ++dd)
                    nc[dd] = accum[static_cast<size_t>(c) * uDim + dd] * inv;
            }
        }

        // ---- Texton histogram: nearest-codebook label per pixel ----------
        std::vector<float> hist(uK, 0.0f);
        for (UINT32 i = 0; i < static_cast<UINT32>(cPix); ++i)
        {
            responseInto(i, resp);
            UINT32 best = 0;
            float bestD = 1e30f;
            for (UINT32 c = 0; c < uK; ++c)
            {
                const float* cen = &codebook[static_cast<size_t>(c) * uDim];
                float d = 0.0f;
                for (UINT32 dd = 0; dd < uDim; ++dd)
                {
                    const float t = resp[dd] - cen[dd];
                    d += t * t;
                }
                if (d < bestD)
                {
                    bestD = d;
                    best = c;
                }
            }
            hist[best] += 1.0f;
        }

        const float fInv = 1.0f / static_cast<float>(cPix);
        for (UINT32 c = 0; c < uK; ++c)
            pFeatures[c] = hist[c] * fInv;

        return S_OK;
    }

private:
    UINT32 m_uNumTextons;
};

// ============================================================================
// CNeuralNet -- simple multi-layer perceptron
// ============================================================================
class CNeuralNet
{
public:
    CNeuralNet()
        : m_uInputSize(0)
        , m_uHiddenSize(0)
        , m_uOutputSize(0)
    {
    }

    ~CNeuralNet() {}

    HRESULT Initialize(UINT32 uInputSize, UINT32 uHiddenSize, UINT32 uOutputSize)
    {
        m_uInputSize = uInputSize;
        m_uHiddenSize = uHiddenSize;
        m_uOutputSize = uOutputSize;

        // Allocate weight matrices
        m_weightsIH.resize(uHiddenSize * uInputSize, 0.0f);
        m_biasH.resize(uHiddenSize, 0.0f);
        m_weightsHO.resize(uOutputSize * uHiddenSize, 0.0f);
        m_biasO.resize(uOutputSize, 0.0f);

        return S_OK;
    }

    HRESULT Forward(const float* pInput, float* pOutput)
    {
        if (!pInput || !pOutput)
            return E_INVALIDARG;

        if (m_uInputSize == 0)
            return E_UNEXPECTED;

        // Hidden layer
        std::vector<float> hidden(m_uHiddenSize, 0.0f);
        for (UINT32 h = 0; h < m_uHiddenSize; ++h)
        {
            float sum = m_biasH[h];
            for (UINT32 i = 0; i < m_uInputSize; ++i)
                sum += pInput[i] * m_weightsIH[h * m_uInputSize + i];

            // Sigmoid activation
            hidden[h] = 1.0f / (1.0f + expf(-sum));
        }

        // Output layer
        for (UINT32 o = 0; o < m_uOutputSize; ++o)
        {
            float sum = m_biasO[o];
            for (UINT32 h = 0; h < m_uHiddenSize; ++h)
                sum += hidden[h] * m_weightsHO[o * m_uHiddenSize + h];

            // Softmax (simplified: just sigmoid for binary)
            pOutput[o] = 1.0f / (1.0f + expf(-sum));
        }

        return S_OK;
    }

    HRESULT LoadWeights(const BYTE* pData, UINT32 cbData)
    {
        if (!pData)
            return E_INVALIDARG;

        // Serialized layout (little-endian):
        //   UINT32 magic   0x4E455457 ("WTEN")
        //   UINT32 version = 1
        //   UINT32 input, hidden, output  (must match Initialize())
        //   float  weightsIH[input * hidden]
        //   float  biasH[hidden]
        //   float  weightsHO[hidden * output]
        //   float  biasO[output]
        const UINT32 kMagic = 0x4E455457u;

        if (cbData < 5 * sizeof(UINT32))
            return E_INVALIDARG;

        UINT32 hdr[5];
        memcpy(hdr, pData, sizeof(hdr));
        if (hdr[0] != kMagic)
            return E_INVALIDARG;
        if (hdr[1] != 1)
            return E_INVALIDARG;

        const UINT32 uIn = hdr[2];
        const UINT32 uHid = hdr[3];
        const UINT32 uOut = hdr[4];
        if (uIn != m_uInputSize || uHid != m_uHiddenSize || uOut != m_uOutputSize)
            return E_INVALIDARG;

        const UINT64 uNumFloats = static_cast<UINT64>(uIn) * uHid + uHid
            + static_cast<UINT64>(uHid) * uOut + uOut;
        if (static_cast<UINT64>(cbData) < 5ull * sizeof(UINT32) + uNumFloats * sizeof(float))
            return E_INVALIDARG;

        const size_t cIH = static_cast<size_t>(uIn) * uHid;
        const size_t cHO = static_cast<size_t>(uHid) * uOut;
        m_weightsIH.resize(cIH);
        m_biasH.resize(uHid);
        m_weightsHO.resize(cHO);
        m_biasO.resize(uOut);

        const BYTE* p = pData + 5 * sizeof(UINT32);
        memcpy(m_weightsIH.data(), p, cIH * sizeof(float));  p += cIH * sizeof(float);
        memcpy(m_biasH.data(), p, uHid * sizeof(float));     p += uHid * sizeof(float);
        memcpy(m_weightsHO.data(), p, cHO * sizeof(float));  p += cHO * sizeof(float);
        memcpy(m_biasO.data(), p, uOut * sizeof(float));

        return S_OK;
    }

private:
    UINT32              m_uInputSize;
    UINT32              m_uHiddenSize;
    UINT32              m_uOutputSize;
    std::vector<float>  m_weightsIH;
    std::vector<float>  m_biasH;
    std::vector<float>  m_weightsHO;
    std::vector<float>  m_biasO;
};

// ============================================================================
// CFaceDetector -- self-contained skin-colour face detector
// ============================================================================
class CFaceDetector
{
public:
    CFaceDetector()
        : m_bInitialized(false)
    {
        ::InitializeSRWLock(&m_lock);
    }

    ~CFaceDetector() {}

    HRESULT Initialize(const WLXDetectorConfig* pConfig)
    {
        if (!pConfig)
            return E_INVALIDARG;

        WLXDetectorConfig cfg = *pConfig;
        // Sanity-check the tuning parameters so the detector never runs with
        // degenerate settings.
        if (cfg.uMinFaceSize < 4)
            cfg.uMinFaceSize = 4;
        if (cfg.dScaleFactor < 1.0)
            cfg.dScaleFactor = 1.0;

        ::AcquireSRWLockExclusive(&m_lock);
        m_config = cfg;
        m_bInitialized = true;
        ::ReleaseSRWLockExclusive(&m_lock);

        return S_OK;
    }

    HRESULT Detect(Gdiplus::Bitmap* pImage, WLXFaceRegion* pRegions, UINT32* pCount)
    {
        if (!pImage || !pRegions || !pCount)
            return E_INVALIDARG;

        UINT32 uMaxFaces = *pCount;
        *pCount = 0;
        if (uMaxFaces == 0)
            return S_OK;

        // Thread safety: the config is written once by Initialize under an
        // exclusive lock and read here under a shared lock; everything after
        // the snapshot operates only on local state, so concurrent Detect
        // calls from multiple threads are safe.
        WLXDetectorConfig cfg;
        {
            ::AcquireSRWLockShared(&m_lock);
            const bool bInit = m_bInitialized;
            if (bInit)
                cfg = m_config;
            ::ReleaseSRWLockShared(&m_lock);
            if (!bInit)
                return E_UNEXPECTED;
        }

        const UINT uImgW = pImage->GetWidth();
        const UINT uImgH = pImage->GetHeight();
        if (uImgW == 0 || uImgH == 0)
            return E_INVALIDARG;

        // ---- 1. GDI+ downscale to a bounded analysis resolution ----------
        // Skin-colour analysis is done on a single downscaled frame. This
        // keeps runtime bounded for 1080p+ video while still resolving the
        // configured minimum face size, which is scaled into work
        // coordinates (the "min-face-size scaling"). dScaleFactor from the
        // config is the Haar-design image-pyramid factor; the single-pass
        // blob detector needs no pyramid, so it is intentionally not used.
        const UINT kAnalysisMaxDim = 640;
        const UINT uMaxDim = (uImgW > uImgH) ? uImgW : uImgH;
        const DOUBLE dScale = (uMaxDim > kAnalysisMaxDim)
            ? static_cast<DOUBLE>(kAnalysisMaxDim) / static_cast<DOUBLE>(uMaxDim)
            : 1.0;

        const UINT uWorkW = (dScale < 1.0)
            ? static_cast<UINT>(std::max(1.0, std::floor(uImgW * dScale + 0.5)))
            : uImgW;
        const UINT uWorkH = (dScale < 1.0)
            ? static_cast<UINT>(std::max(1.0, std::floor(uImgH * dScale + 0.5)))
            : uImgH;

        std::unique_ptr<Gdiplus::Bitmap> pWork(
            new Gdiplus::Bitmap(uWorkW, uWorkH, PixelFormat24bppRGB));
        if (pWork->GetLastStatus() != Gdiplus::Ok)
            return Base::GdiplusStatusToHresult(pWork->GetLastStatus());

        {
            Gdiplus::Graphics g(pWork.get());
            g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
            g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
            const Gdiplus::Status st = g.DrawImage(pImage,
                Gdiplus::Rect(0, 0, static_cast<INT>(uWorkW), static_cast<INT>(uWorkH)));
            if (st != Gdiplus::Ok)
                return Base::GdiplusStatusToHresult(st);
        }

        // ---- 2. Raw BGR pixels -------------------------------------------
        Gdiplus::BitmapData bd;
        Gdiplus::Rect rc(0, 0, static_cast<INT>(uWorkW), static_cast<INT>(uWorkH));
        if (pWork->LockBits(&rc, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &bd) != Gdiplus::Ok)
            return E_FAIL;
        const BYTE* pPixels = static_cast<const BYTE*>(bd.Scan0);
        const INT lStride = bd.Stride;

        // ---- 3. Skin-colour mask (documented rule set) -------------------
        // A pixel is skin if EITHER published rule fires:
        //   (a) YCbCr box  (Chai & Ngan, IEEE Trans. CSVT 9(1) 1999):
        //       77 <= Cb <= 127 and 133 <= Cr <= 173 using BT.601 conversion.
        //   (b) normalized-RGB rule (Peer et al., Proc. EURASIP 2003):
        //       R>95, G>40, B>20, R>=G>=B, (R-G)>15, (max-min)>15.
        const size_t cPix = static_cast<size_t>(uWorkW) * uWorkH;
        std::vector<BYTE> mask(cPix, 0);

        for (UINT y = 0; y < uWorkH; ++y)
        {
            const BYTE* row = pPixels + static_cast<INT>(y) * lStride;
            for (UINT x = 0; x < uWorkW; ++x)
            {
                const BYTE b = row[x * 3 + 0];
                const BYTE g = row[x * 3 + 1];
                const BYTE r = row[x * 3 + 2];

                const INT cb = (( -38 * r - 74 * g + 112 * b) >> 8) + 128;
                const INT cr = (( 112 * r - 94 * g -  18 * b) >> 8) + 128;
                const bool bYcc = (cb >= 77 && cb <= 127 && cr >= 133 && cr <= 173);

                const INT iMax = std::max(r, std::max(g, b));
                const INT iMin = std::min(r, std::min(g, b));
                const bool bRgb = (r > 95 && g > 40 && b > 20 &&
                                   r >= g && r >= b &&
                                   (r - g) > 15 && (iMax - iMin) > 15);

                mask[static_cast<size_t>(y) * uWorkW + x] = (bYcc || bRgb) ? 1 : 0;
            }
        }

        // Single 3x3 majority pass closes small facial gaps (eyes, brows,
        // mouth) so one face is not fragmented into several blobs.
        std::vector<BYTE> closed(mask);
        for (UINT y = 1; y + 1 < uWorkH; ++y)
        {
            for (UINT x = 1; x + 1 < uWorkW; ++x)
            {
                UINT cnt = 0;
                for (INT dy = -1; dy <= 1; ++dy)
                    for (INT dx = -1; dx <= 1; ++dx)
                        cnt += mask[static_cast<size_t>(y + dy) * uWorkW + (x + dx)];
                closed[static_cast<size_t>(y) * uWorkW + x] = (cnt >= 5) ? 1 : 0;
            }
        }

        pWork->UnlockBits(&bd);

        // ---- 4. Connected components (union-find) ------------------------
        // parent[]: -2 = no pixel, -1 = own root, >=0 = unioned.
        std::vector<INT> parent(cPix, -2);
        const auto findRoot = [&parent](INT i) -> INT
        {
            INT root = i;
            while (parent[root] >= 0)
                root = parent[root];
            while (parent[i] >= 0)
            {
                const INT nxt = parent[i];
                parent[i] = root;
                i = nxt;
            }
            return root;
        };
        const auto doUnion = [&parent, &findRoot](INT a, INT b) -> void
        {
            INT ra = findRoot(a);
            INT rb = findRoot(b);
            if (ra == rb)
                return;
            if (parent[ra] > parent[rb])
                std::swap(ra, rb); // ra holds the larger (more negative) size
            parent[ra] += parent[rb];
            parent[rb] = ra;
        };

        for (UINT y = 0; y < uWorkH; ++y)
        {
            for (UINT x = 0; x < uWorkW; ++x)
            {
                const INT idx = static_cast<INT>(static_cast<size_t>(y) * uWorkW + x);
                if (closed[idx] == 0)
                    continue;
                parent[idx] = -1;
                if (x > 0 && closed[static_cast<size_t>(y) * uWorkW + (x - 1)])
                    doUnion(idx - 1, idx);
                if (y > 0 && closed[static_cast<size_t>(y - 1) * uWorkW + x])
                    doUnion(idx - static_cast<INT>(uWorkW), idx);
            }
        }

        // ---- 5. Per-component statistics ---------------------------------
        struct CompStats { INT minX, minY, maxX, maxY; UINT32 count; };
        std::vector<CompStats> comps;
        std::vector<INT> compIndex(cPix, -1);

        for (UINT y = 0; y < uWorkH; ++y)
        {
            for (UINT x = 0; x < uWorkW; ++x)
            {
                const INT idx = static_cast<INT>(static_cast<size_t>(y) * uWorkW + x);
                if (parent[idx] == -2)
                    continue;
                const INT root = findRoot(idx);
                INT ci = compIndex[root];
                if (ci < 0)
                {
                    ci = static_cast<INT>(comps.size());
                    CompStats st = { std::numeric_limits<INT>::max(),
                                     std::numeric_limits<INT>::max(), -1, -1, 0 };
                    comps.push_back(st);
                    compIndex[root] = ci;
                }
                CompStats& st = comps[static_cast<size_t>(ci)];
                const INT ix = static_cast<INT>(x);
                const INT iy = static_cast<INT>(y);
                if (ix < st.minX) st.minX = ix;
                if (iy < st.minY) st.minY = iy;
                if (ix > st.maxX) st.maxX = ix;
                if (iy > st.maxY) st.maxY = iy;
                ++st.count;
            }
        }

        // ---- 6. Candidate filtering --------------------------------------
        // Minimum face size, scaled from original pixels into the analysis
        // resolution (uMinFaceSize/uMaxFaceSize are applied again in
        // original coordinates after mapping back below).
        const INT iWorkMin = std::max(4, static_cast<INT>(std::ceil(
            static_cast<DOUBLE>(cfg.uMinFaceSize) * dScale)));
        const INT iWorkMax = (cfg.uMaxFaceSize > 0)
            ? std::max(iWorkMin, static_cast<INT>(std::ceil(
                static_cast<DOUBLE>(cfg.uMaxFaceSize) * dScale)))
            : 0;

        struct Candidate { DOUBLE x, y, cx, cy, conf; };
        std::vector<Candidate> cands;

        for (size_t c = 0; c < comps.size(); ++c)
        {
            const CompStats& st = comps[c];
            if (st.count < static_cast<UINT32>(iWorkMin) * static_cast<UINT32>(iWorkMin) / 4u)
                continue;

            const INT bw = st.maxX - st.minX + 1;
            const INT bh = st.maxY - st.minY + 1;
            if (bw < iWorkMin || bh < iWorkMin)
                continue;
            if (iWorkMax > 0 && (bw > iWorkMax || bh > iWorkMax))
                continue;

            // Faces are roughly portrait-shaped.
            const DOUBLE dAspect = static_cast<DOUBLE>(bh) / static_cast<DOUBLE>(std::max(1, bw));
            if (dAspect < 0.45 || dAspect > 2.5)
                continue;

            // Too sparse = fragmented noise; too dense = skin-toned surface.
            const DOUBLE dFill = static_cast<DOUBLE>(st.count)
                / static_cast<DOUBLE>(bw * bh);
            if (dFill < 0.15 || dFill > 0.92)
                continue;

            // A blob covering most of the frame is a background plane, not a
            // face (e.g. a skin-toned wall filling the shot).
            const DOUBLE dFrameCover = static_cast<DOUBLE>(bw * bh)
                / static_cast<DOUBLE>(uWorkW * uWorkH);
            if (dFrameCover > 0.6)
                continue;

            // Quality score: weighted combination of fill, size and aspect.
            const DOUBLE dAreaFrac = std::min(dFrameCover, 0.5) / 0.5;
            const DOUBLE dSizeNorm = 1.0 - std::fabs(dAreaFrac - 0.3) / 0.3;
            const DOUBLE dAspectNorm = 1.0 - std::fabs(dAspect - 1.25) / 1.25;
            DOUBLE dConf = 0.45 * std::min(std::max((dFill - 0.15) / 0.55, 0.0), 1.0)
                + 0.35 * std::min(std::max(dSizeNorm, 0.0), 1.0)
                + 0.20 * std::min(std::max(dAspectNorm, 0.0), 1.0);
            const DOUBLE dConfFinal = std::min(std::max(dConf, 0.05), 0.95) * 0.9;

            // Map back to original coordinates and expand ~10% so the box
            // frames hair/chin context for the auto-frame feature.
            const DOUBLE dScaleInv = 1.0 / dScale;
            DOUBLE dX = st.minX * dScaleInv;
            DOUBLE dY = st.minY * dScaleInv;
            DOUBLE dCx = bw * dScaleInv;
            DOUBLE dCy = bh * dScaleInv;

            dX -= dCx * 0.05;
            dY -= dCy * 0.05;
            dCx *= 1.1;
            dCy *= 1.1;

            // Re-apply the min/max face size in original coordinates.
            if (dCx < cfg.uMinFaceSize || dCy < cfg.uMinFaceSize)
                continue;
            if (cfg.uMaxFaceSize > 0 && (dCx > cfg.uMaxFaceSize || dCy > cfg.uMaxFaceSize))
                continue;

            // Clamp to the frame.
            dX = std::max(0.0, dX);
            dY = std::max(0.0, dY);
            if (dX + dCx > uImgW) dCx = uImgW - dX;
            if (dY + dCy > uImgH) dCy = uImgH - dY;
            if (dCx < 1 || dCy < 1)
                continue;

            Candidate cd = { dX, dY, dCx, dCy, dConfFinal };
            cands.push_back(cd);
        }

        // ---- 7. Non-maximum suppression (IoU) ----------------------------
        // Strongest candidate first; a box is kept unless it overlaps an
        // already-kept box by more than 45%.
        std::sort(cands.begin(), cands.end(),
            [](const Candidate& a, const Candidate& b) { return a.conf > b.conf; });

        std::vector<Candidate> kept;
        for (size_t i = 0; i < cands.size(); ++i)
        {
            bool bDup = false;
            for (size_t j = 0; j < kept.size() && !bDup; ++j)
            {
                const Candidate& a = cands[i];
                const Candidate& b = kept[j];
                const DOUBLE ix1 = std::max(a.x, b.x);
                const DOUBLE iy1 = std::max(a.y, b.y);
                const DOUBLE ix2 = std::min(a.x + a.cx, b.x + b.cx);
                const DOUBLE iy2 = std::min(a.y + a.cy, b.y + b.cy);
                const DOUBLE iw = std::max(0.0, ix2 - ix1);
                const DOUBLE ih = std::max(0.0, iy2 - iy1);
                const DOUBLE dDen = a.cx * a.cy + b.cx * b.cy - iw * ih;
                const DOUBLE dIoU = (dDen > 0.0) ? (iw * ih) / dDen : 0.0;
                if (dIoU > 0.45)
                    bDup = true;
            }
            if (!bDup)
                kept.push_back(cands[i]);
        }

        // ---- 8. Emit results ---------------------------------------------
        const UINT32 uOut = static_cast<UINT32>(
            std::min(kept.size(), static_cast<size_t>(uMaxFaces)));
        for (UINT32 i = 0; i < uOut; ++i)
        {
            WLXFaceRegion& r = pRegions[i];
            r.x = static_cast<INT>(std::floor(kept[i].x));
            r.y = static_cast<INT>(std::floor(kept[i].y));
            r.cx = static_cast<INT>(std::ceil(kept[i].cx));
            r.cy = static_cast<INT>(std::ceil(kept[i].cy));
            r.dConfidence = kept[i].conf;
            r.uFaceId = i + 1;
        }
        *pCount = uOut;

        return S_OK;
    }

private:
    bool                m_bInitialized;
    WLXDetectorConfig   m_config;
    SRWLOCK             m_lock;
};

// ============================================================================
// CFaceRecognizor -- face identity recognition
// ============================================================================
class CFaceRecognizor
{
public:
    CFaceRecognizor()
        : m_bInitialized(false)
    {
    }

    ~CFaceRecognizor() {}

    HRESULT Initialize()
    {
        // Initialize feature extractors
        m_lbpExtractor.reset(new CLBPFeatureExtractor());
        m_pcaExtractor.reset(new PCAFeatureExtractor());
        m_textonExtractor.reset(new CTextonFeatureExtractor());

        // Initialize neural network classifier
        m_neuralNet.reset(new CNeuralNet());
        // 256 LBP features -> 128 hidden -> 1 output
        m_neuralNet->Initialize(256, 128, 1);
        m_uNetOutput = 1;

        m_bInitialized = true;
        return S_OK;
    }

    HRESULT Recognize(Gdiplus::Bitmap* pImage, const WLXFaceRegion* pFace,
        WLXRecognitionResult* pResult)
    {
        if (!m_bInitialized || !pImage || !pFace || !pResult)
            return E_UNEXPECTED;

        // Real recognition pipeline: crop -> normalize to 64x64 ->
        // grayscale -> LBP histogram -> MLP forward pass. The DLL ships no
        // enrolled identity database, so a match can never be flagged
        // "known"; the match score is still a genuinely computed scalar
        // (the neural net output for the face patch).
        *pResult = WLXRecognitionResult();

        const UINT uImgW = pImage->GetWidth();
        const UINT uImgH = pImage->GetHeight();
        if (uImgW == 0 || uImgH == 0)
            return E_INVALIDARG;

        const INT iCx = ClampInt(pFace->x, 0, static_cast<INT>(uImgW) - 1);
        const INT iCy = ClampInt(pFace->y, 0, static_cast<INT>(uImgH) - 1);
        const INT iCw = ClampInt(pFace->cx, 1, static_cast<INT>(uImgW) - iCx);
        const INT iCh = ClampInt(pFace->cy, 1, static_cast<INT>(uImgH) - iCy);

        // Crop the face region.
        std::unique_ptr<Gdiplus::Bitmap> pCrop(
            new Gdiplus::Bitmap(iCw, iCh, PixelFormat24bppRGB));
        if (pCrop->GetLastStatus() != Gdiplus::Ok)
            return Base::GdiplusStatusToHresult(pCrop->GetLastStatus());
        {
            Gdiplus::Graphics g(pCrop.get());
            const Gdiplus::Status st = g.DrawImage(pImage,
                Gdiplus::Rect(0, 0, iCw, iCh), iCx, iCy, iCw, iCh, Gdiplus::UnitPixel);
            if (st != Gdiplus::Ok)
                return Base::GdiplusStatusToHresult(st);
        }

        // Normalize to a standard 64x64 patch.
        const UINT kFaceNorm = 64;
        std::unique_ptr<Gdiplus::Bitmap> pFaceImg(
            new Gdiplus::Bitmap(kFaceNorm, kFaceNorm, PixelFormat24bppRGB));
        if (pFaceImg->GetLastStatus() != Gdiplus::Ok)
            return Base::GdiplusStatusToHresult(pFaceImg->GetLastStatus());
        {
            Gdiplus::Graphics g(pFaceImg.get());
            g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
            const Gdiplus::Status st = g.DrawImage(pCrop.get(),
                Gdiplus::Rect(0, 0, static_cast<INT>(kFaceNorm), static_cast<INT>(kFaceNorm)));
            if (st != Gdiplus::Ok)
                return Base::GdiplusStatusToHresult(st);
        }

        // Convert to luminance (BT.601) for the LBP extractor.
        std::vector<BYTE> lum(static_cast<size_t>(kFaceNorm) * kFaceNorm);
        {
            Gdiplus::BitmapData bd;
            Gdiplus::Rect rc(0, 0, static_cast<INT>(kFaceNorm), static_cast<INT>(kFaceNorm));
            if (pFaceImg->LockBits(&rc, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &bd) != Gdiplus::Ok)
                return E_FAIL;
            const BYTE* p = static_cast<const BYTE*>(bd.Scan0);
            const INT lStride = bd.Stride;
            for (UINT y = 0; y < kFaceNorm; ++y)
            {
                const BYTE* row = p + static_cast<INT>(y) * lStride;
                for (UINT x = 0; x < kFaceNorm; ++x)
                {
                    const INT l = (static_cast<INT>(row[x * 3 + 2]) * 77
                        + static_cast<INT>(row[x * 3 + 1]) * 150
                        + static_cast<INT>(row[x * 3]) * 29) >> 8;
                    lum[static_cast<size_t>(y) * kFaceNorm + x] = static_cast<BYTE>(l);
                }
            }
            pFaceImg->UnlockBits(&bd);
        }

        // LBP histogram (256 bins) -> MLP forward pass.
        std::vector<float> feats(256, 0.0f);
        UINT32 uFeatSize = 0;
        HRESULT hr = m_lbpExtractor->Extract(lum.data(), kFaceNorm, kFaceNorm,
            feats.data(), &uFeatSize);
        if (FAILED(hr))
            return hr;

        std::vector<float> out(m_uNetOutput, 0.0f);
        hr = m_neuralNet->Forward(feats.data(), out.data());
        if (FAILED(hr))
            return hr;

        // Net output is a sigmoid in [0,1]; near 0.5 the (untrained) model
        // has no opinion, near 1.0 it reports a confident "known" answer.
        pResult->uFaceId = pFace->uFaceId;
        pResult->dMatchScore = ClampF(out[0], 0.0f, 1.0f);
        pResult->bIsKnown = FALSE;   // no enrolled identity database in the DLL
        pResult->uPersonId = 0;

        return S_OK;
    }

private:
    bool                                    m_bInitialized;
    UINT32                                  m_uNetOutput;
    std::unique_ptr<CLBPFeatureExtractor>   m_lbpExtractor;
    std::unique_ptr<PCAFeatureExtractor>    m_pcaExtractor;
    std::unique_ptr<CTextonFeatureExtractor> m_textonExtractor;
    std::unique_ptr<CNeuralNet>             m_neuralNet;
};

} // namespace LiveLabs

// ============================================================================
// Module state
// ============================================================================
// GDI+/COM startup/shutdown is owned by dllmain.cpp (DllMain); this file keeps
// no module-level state so it is free of cross-thread globals.

// ============================================================================
// Exported functions -- standard COM DLL entry points
// ============================================================================

extern "C"
{

// This DLL creates no class objects and keeps no instances, so it is always
// safe to unload once GetClassObject has never been satisfied.
STDAPI DllCanUnloadNow()
{
    return S_OK;
}

// No COM class factory is registered by this DLL (face detection is exposed
// through the LiveLabs classes consumed in-process), so the request is
// refused rather than pretending a factory exists.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(ppv);
    return CLASS_E_CLASSNOTAVAILABLE;
}

// There are no COM classes to register or unregister, so both operations are
// no-ops by design (returning S_OK is accurate, not a stub).
STDAPI DllRegisterServer()
{
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    return S_OK;
}

} // extern "C"
