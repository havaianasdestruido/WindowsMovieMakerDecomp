#include "pch.h"
// PatternMesh.cpp - Wipe transition pattern mesh implementation

#include "PatternMesh.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace HMREngine
{
namespace DX
{

// ============================================================================
// PatternMesh base
// ============================================================================
PatternMesh::PatternMesh() = default;
PatternMesh::~PatternMesh() { Release(); }

HRESULT PatternMesh::Generate(ID3D11Device* dev, UINT width, UINT height)
{
    if (!dev) return E_POINTER;

    std::vector<PatternVertex> vertices;
    std::vector<WORD> indices;

    BuildVertices(vertices, width, height);
    BuildIndices(indices);

    return CreateBuffers(dev, vertices, indices);
}

HRESULT PatternMesh::Generate(MeshResourceDX* mesh, UINT width, UINT height)
{
    if (!mesh) return E_POINTER;

    std::vector<PatternVertex> vertices;
    std::vector<WORD> indices;

    BuildVertices(vertices, width, height);
    BuildIndices(indices);

    return mesh->CreateFromData(vertices.data(), static_cast<UINT>(vertices.size()),
        sizeof(PatternVertex), indices.data(), static_cast<UINT>(indices.size()));
}

void PatternMesh::Release()
{
    m_indexBuffer.Release();
    m_vertexBuffer.Release();
    m_vertexCount = 0;
    m_indexCount = 0;
}

HRESULT PatternMesh::CreateBuffers(ID3D11Device* dev, const std::vector<PatternVertex>& vertices,
    const std::vector<WORD>& indices)
{
    Release();

    m_vertexCount = static_cast<UINT>(vertices.size());
    m_indexCount = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(PatternVertex));
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();

    HRESULT hr = dev->CreateBuffer(&bd, &initData, &m_vertexBuffer);
    if (FAILED(hr)) return hr;

    bd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(WORD));
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indices.data();

    return dev->CreateBuffer(&bd, &initData, &m_indexBuffer);
}

float PatternMesh::ApplySoftEdge(float mask) const
{
    if (m_softEdge <= 0.0f) return mask > 0.5f ? 1.0f : 0.0f;
    float half = m_softEdge * 0.5f;
    return Saturate((mask - (0.5f - half)) / m_softEdge);
}

float PatternMesh::ComputeRadialMask(float x, float y, float cx, float cy, float radius) const
{
    float dx = x - cx;
    float dy = y - cy;
    float dist = sqrtf(dx * dx + dy * dy);
    return Saturate(1.0f - dist / radius);
}

float PatternMesh::ComputeAngularMask(float x, float y, float cx, float cy,
    float startAngle, float sweep) const
{
    float dx = x - cx;
    float dy = y - cy;
    float angle = atan2f(dy, dx);
    if (angle < 0) angle += 2.0f * (float)M_PI;
    if (angle < startAngle) angle += 2.0f * (float)M_PI;

    float endAngle = startAngle + sweep;
    if (sweep >= 2.0f * (float)M_PI) return 1.0f;

    if (angle >= startAngle && angle <= endAngle)
        return Saturate((angle - startAngle) / sweep);
    return 0.0f;
}

// ============================================================================
// CheckerboardPatternMesh
// ============================================================================
void CheckerboardPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    float cellW = 2.0f / m_gridSize;
    float cellH = 2.0f / m_gridSize;

    for (UINT y = 0; y < m_gridSize; ++y)
    {
        for (UINT x = 0; x < m_gridSize; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            bool isOdd = ((x + y) % 2) == 1;
            float mask = isOdd ? m_progress * 2.0f : m_progress * 2.0f - 1.0f;
            mask = Saturate(mask);

            float u0 = (float)x / m_gridSize;
            float v0 = (float)y / m_gridSize;
            float u1 = (float)(x + 1) / m_gridSize;
            float v1 = (float)(y + 1) / m_gridSize;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = ApplySoftEdge(mask); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0);
            v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1);
            v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1);
            v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void CheckerboardPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD quadCount = m_gridSize * m_gridSize;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
}

// ============================================================================
// BowTiePatternMesh
// ============================================================================
void BowTiePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 12;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float nx = fabsf(px);
                float ny = fabsf(py);
                float bowtie = (nx > 0.001f) ? (ny / nx) : 1.0f;
                float shapeMask = Saturate(bowtie - (1.0f - m_progress) * 2.0f + 1.0f);
                return ApplySoftEdge(shapeMask);
            };

            float u0 = (float)x / tess;
            float v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess;
            float v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void BowTiePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 12;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// DiagonalCrossPatternMesh
// ============================================================================
void DiagonalCrossPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float diag1 = fabsf(px - py);
                float diag2 = fabsf(px + py);
                float crossDist = (std::min)(diag1, diag2);
                float width = m_progress * 2.0f;
                float mask = Saturate(width - crossDist);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void DiagonalCrossPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// DiagonalBoxPatternMesh
// ============================================================================
void DiagonalBoxPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float diag = fabsf(px - py);
                float boxSize = m_progress * 2.8f;
                float mask = Saturate(boxSize - diag);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void DiagonalBoxPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// SplitPatternMesh
// ============================================================================
void SplitPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float halfGap = (1.0f - m_progress) * 1.0f;
                if (py > 0)
                    return ApplySoftEdge(Saturate(px + halfGap));
                else
                    return ApplySoftEdge(Saturate(-px + halfGap));
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void SplitPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// RevealPatternMesh
// ============================================================================
void RevealPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float normalizedX = (px + 1.0f) * 0.5f;
                float normalizedY = (py + 1.0f) * 0.5f;
                float edgeDist = (std::max)(fabsf(normalizedX - 0.5f), fabsf(normalizedY - 0.5f));
                return ApplySoftEdge(Saturate(m_progress * 1.5f - edgeDist + 0.1f));
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void RevealPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// CirclesPatternMesh
// ============================================================================
void CirclesPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    struct CircleCenter { float x, y; float delay; };
    std::vector<CircleCenter> centers;
    float goldenAngle = 2.399963f;
    for (UINT i = 0; i < m_numCircles; ++i)
    {
        float t = (float)i / m_numCircles;
        float r = sqrtf(t) * 0.85f;
        float angle = i * goldenAngle;
        centers.push_back({ r * cosf(angle), r * sinf(angle), t * 0.4f });
    }

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float bestMask = 0.0f;
                for (auto& c : centers)
                {
                    float dist = sqrtf((px - c.x) * (px - c.x) + (py - c.y) * (py - c.y));
                    float localProg = Saturate(m_progress * 1.5f - c.delay);
                    float radius = localProg * 0.35f;
                    float circleMask = Saturate(1.0f - dist / radius);
                    bestMask = (std::max)(bestMask, circleMask);
                }
                return ApplySoftEdge(bestMask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void CirclesPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// CirclePatternMesh
// ============================================================================
void CirclePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;
    float radius = m_progress * 1.4142f;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float dist = sqrtf(px * px + py * py);
                float mask = Saturate(1.0f - (dist / radius));
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void CirclePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// FanAndSweepPatternMesh
// ============================================================================
void FanAndSweepPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float angle = atan2f(py, px);
                if (angle < 0) angle += 2.0f * 3.14159265f;
                float sweepAngle = m_progress * 2.0f * 3.14159265f;
                float dist = sqrtf(px * px + py * py);
                float angularMask = (angle <= sweepAngle) ? 1.0f : 0.0f;
                if (sweepAngle > 0.0f && sweepAngle < 2.0f * 3.14159265f)
                {
                    float edgeDist = fabsf(angle - sweepAngle);
                    if (edgeDist < 0.15f)
                        angularMask = Saturate((sweepAngle - angle + 0.15f) / 0.15f);
                }
                float radialMask = Saturate(dist / 0.1f);
                float mask = angularMask * radialMask;
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void FanAndSweepPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// FanAndSweepBasePatternMesh
// ============================================================================
void FanAndSweepBasePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float angle = atan2f(py + 1.0f, px);
                if (angle < 0) angle += 2.0f * 3.14159265f;
                float sweepAngle = m_progress * 3.14159265f;
                float angularMask = (angle <= sweepAngle) ? 1.0f : 0.0f;
                if (sweepAngle > 0.0f && sweepAngle < 3.14159265f)
                {
                    float edgeDist = fabsf(angle - sweepAngle);
                    if (edgeDist < 0.2f)
                        angularMask = Saturate((sweepAngle - angle + 0.2f) / 0.2f);
                }
                return ApplySoftEdge(angularMask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void FanAndSweepBasePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// FanInPatternMesh
// ============================================================================
void FanInPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float angle = atan2f(py, px);
                if (angle < 0) angle += 2.0f * 3.14159265f;
                float sweepAngle = m_progress * 2.0f * 3.14159265f;
                float angleDiff = fmodf(angle, 2.0f * 3.14159265f);
                float mask;
                if (sweepAngle >= 2.0f * 3.14159265f)
                    mask = 1.0f;
                else
                    mask = (angleDiff <= sweepAngle) ? 1.0f : 0.0f;
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void FanInPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// EyePatternMesh
// ============================================================================
void EyePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float ex = px * 2.0f;
                float ey = py * 3.0f;
                float dist = sqrtf(ex * ex + ey * ey);
                float radius = m_progress * 2.5f;
                float mask = Saturate(1.0f - (dist / radius));
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void EyePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// FanUpPatternMesh
// ============================================================================
void FanUpPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float ny = (py + 1.0f) * 0.5f;
                float nx = fabsf(px);
                float wedge = nx * (1.0f - m_progress) * 3.0f;
                float mask = Saturate(ny - wedge);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void FanUpPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// FanOutPatternMesh
// ============================================================================
void FanOutPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float dist = sqrtf(px * px + py * py);
                float maxDist = 1.4142f;
                float mask = Saturate(m_progress * maxDist * 1.5f - dist);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void FanOutPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// HeartPatternMesh
// ============================================================================
void HeartPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 20;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float nx = px * 1.2f;
                float ny = -py + 0.3f;
                float x2 = nx * nx;
                float y2 = ny * ny;
                float d = powf(x2 + y2 - 1.0f, 3.0f) - x2 * y2 * ny;
                float inside = (d < 0.0f) ? 1.0f : 0.0f;
                float threshold = (m_progress - 0.5f) * 2.0f;
                float mask = Saturate(inside + threshold);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void HeartPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 20;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// FillVPatternMesh
// ============================================================================
void FillVPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 8;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float py) -> float {
                float normalizedY = (py + 1.0f) * 0.5f;
                return ApplySoftEdge(Saturate(m_progress * 1.2f - normalizedY + 0.1f));
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void FillVPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 8;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// KeyholePatternMesh
// ============================================================================
void KeyholePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float circleMask = 0.0f;
                float cx = px, cy = py - 0.3f;
                float circleDist = sqrtf(cx * cx + cy * cy);
                if (circleDist < m_progress * 0.5f)
                    circleMask = 1.0f;

                float slotMask = 0.0f;
                float slotX = fabsf(px);
                float slotY = py + 0.3f;
                if (slotX < m_progress * 0.15f && slotY > 0.0f && slotY < m_progress * 0.8f)
                    slotMask = 1.0f;

                float mask = (std::max)(circleMask, slotMask);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void KeyholePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// IrisPatternMesh
// ============================================================================
void IrisPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;
    const int numBlades = 6;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float dist = sqrtf(px * px + py * py);
                float angle = atan2f(py, px);
                if (angle < 0) angle += 2.0f * 3.14159265f;

                float bladeAngle = angle * numBlades;
                float bladeFactor = fabsf(sinf(bladeAngle * 0.5f));
                float maxRadius = m_progress * 1.8f;
                float adjustedRadius = maxRadius * (0.7f + 0.3f * bladeFactor);
                float mask = Saturate(1.0f - dist / adjustedRadius);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void IrisPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// RectanglesPatternMesh
// ============================================================================
void RectanglesPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    float cellW = 2.0f / m_cols;
    float cellH = 2.0f / m_rows;

    for (UINT y = 0; y < m_rows; ++y)
    {
        for (UINT x = 0; x < m_cols; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            float cx = (left + right) * 0.5f;
            float cy = (top + bottom) * 0.5f;
            float dist = sqrtf(cx * cx + cy * cy);
            float delay = dist * 0.5f;
            float localProgress = Saturate(m_progress * 2.0f - delay);
            float mask = ApplySoftEdge(localProgress);

            float u0 = (float)x / m_cols;
            float v0 = (float)y / m_rows;
            float u1 = (float)(x + 1) / m_cols;
            float v1 = (float)(y + 1) / m_rows;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = mask; v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0);
            v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1);
            v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1);
            v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void RectanglesPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD quadCount = m_cols * m_rows;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// RectanglePatternMesh
// ============================================================================
void RectanglePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 12;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float nx = fabsf(px);
                float ny = fabsf(py);
                float maxEdge = (std::max)(nx, ny);
                float mask = Saturate(m_progress * 1.5f - maxEdge + 0.1f);
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void RectanglePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 12;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// StarsPatternMesh
// ============================================================================
void StarsPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    struct StarPos { float x, y; float delay; };
    std::vector<StarPos> stars;
    float goldenAngle = 2.399963f;
    for (UINT i = 0; i < m_numStars; ++i)
    {
        float t = (float)i / m_numStars;
        float r = sqrtf(t) * 0.8f;
        float angle = i * goldenAngle;
        stars.push_back({ r * cosf(angle), r * sinf(angle), t * 0.5f });
    }

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float bestMask = 0.0f;
                for (auto& s : stars)
                {
                    float dx = px - s.x;
                    float dy = py - s.y;
                    float angle = atan2f(dy, dx);
                    float dist = sqrtf(dx * dx + dy * dy);
                    float starAngle = angle * 5.0f;
                    float spike = fabsf(cosf(starAngle));
                    float maxRadius = 0.25f;
                    float localProg = Saturate(m_progress * 1.5f - s.delay);
                    float radius = localProg * maxRadius * (0.5f + 0.5f * spike);
                    float mask = Saturate(1.0f - dist / radius);
                    bestMask = (std::max)(bestMask, mask);
                }
                return ApplySoftEdge(bestMask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void StarsPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// StarPatternMesh
// ============================================================================
void StarPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;
    const int numPoints = 5;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float dist = sqrtf(px * px + py * py);
                float angle = atan2f(py, px);
                if (angle < 0) angle += 2.0f * 3.14159265f;

                float starAngle = angle * numPoints;
                float spike = fabsf(cosf(starAngle));
                float innerRadius = 0.3f;
                float outerRadius = 1.0f;
                float maxRadius = m_progress * outerRadius;
                float radiusAtAngle = innerRadius + (outerRadius - innerRadius) * spike;
                float mask = Saturate(1.0f - dist / (maxRadius * radiusAtAngle));
                return ApplySoftEdge(mask);
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void StarPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// SweepInPatternMesh
// ============================================================================
void SweepInPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px) -> float {
                float normalizedX = (px + 1.0f) * 0.5f;
                return ApplySoftEdge(Saturate(m_progress * 1.2f - normalizedX + 0.1f));
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void SweepInPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// SweepOutPatternMesh
// ============================================================================
void SweepOutPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float px, float py) -> float {
                float normalizedX = fabsf(px);
                float normalizedY = fabsf(py);
                float edgeDist = (std::max)(normalizedX, normalizedY);
                return ApplySoftEdge(Saturate(m_progress * 1.5f - edgeDist + 0.1f));
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(left, top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(right, top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(left, bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(right, bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void SweepOutPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// WheelPatternMesh
// ============================================================================
void WheelPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    float anglePerSpoke = 2.0f * (float)M_PI / m_spokes;

    vertices.clear();
    PatternVertex center;
    center.position = Vec3(0, 0, 0);
    center.texCoord = Vec2(0.5f, 0.5f);
    center.mask = ApplySoftEdge(m_progress);
    center.feather = 0; center.padding = 0;
    vertices.push_back(center);

    for (UINT i = 0; i < m_spokes; ++i)
    {
        float angle = i * anglePerSpoke;
        float x = cosf(angle);
        float y = sinf(angle);

        float spokeProgress = Saturate(m_progress * m_spokes - (float)i);
        PatternVertex v;
        v.position = Vec3(x, y, 0);
        v.texCoord = Vec2(x * 0.5f + 0.5f, y * 0.5f + 0.5f);
        v.mask = ApplySoftEdge(spokeProgress);
        v.feather = 0; v.padding = 0;
        vertices.push_back(v);
    }
}

void WheelPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    for (WORD i = 1; i <= m_spokes; ++i)
    {
        WORD next = (i % m_spokes) + 1;
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(next);
    }
}

// ============================================================================
// SweepUpPatternMesh
// ============================================================================
void SweepUpPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    const UINT tess = 16;
    float cellW = 2.0f / tess;
    float cellH = 2.0f / tess;

    for (UINT y = 0; y < tess; ++y)
    {
        for (UINT x = 0; x < tess; ++x)
        {
            float left = -1.0f + x * cellW;
            float top = -1.0f + y * cellH;
            float right = left + cellW;
            float bottom = top + cellH;

            auto computeMask = [&](float py) -> float {
                float normalizedY = (py + 1.0f) * 0.5f;
                return ApplySoftEdge(Saturate(m_progress * 1.2f - normalizedY + 0.1f));
            };

            float u0 = (float)x / tess, v0 = (float)y / tess;
            float u1 = (float)(x + 1) / tess, v1 = (float)(y + 1) / tess;

            PatternVertex v;
            v.texCoord = Vec2(u0, v0); v.mask = computeMask(top); v.feather = 0; v.padding = 0;
            v.position = Vec3(left, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v0); v.mask = computeMask(top); v.position = Vec3(right, top, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u0, v1); v.mask = computeMask(bottom); v.position = Vec3(left, bottom, 0.0f); vertices.push_back(v);
            v.texCoord = Vec2(u1, v1); v.mask = computeMask(bottom); v.position = Vec3(right, bottom, 0.0f); vertices.push_back(v);
        }
    }
}

void SweepUpPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    WORD tess = 16;
    WORD quadCount = tess * tess;
    for (WORD q = 0; q < quadCount; ++q)
    {
        WORD base = q * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

// ============================================================================
// ZigzagPatternMesh
// ============================================================================
void ZigzagPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    float rowH = 2.0f / m_zigzagRows;

    for (UINT y = 0; y < m_zigzagRows; ++y)
    {
        float top = -1.0f + y * rowH;
        float bottom = top + rowH;
        bool oddRow = (y % 2) == 1;

        float left = oddRow ? 0.0f : -1.0f;
        float right = oddRow ? 1.0f : 0.0f;

        float localProgress = Saturate(m_progress * 2.0f - (float)y / m_zigzagRows);
        float mask = ApplySoftEdge(localProgress);

        float v0 = (float)y / m_zigzagRows;
        float v1 = (float)(y + 1) / m_zigzagRows;

        PatternVertex v;
        v.mask = mask; v.feather = 0; v.padding = 0;

        v.position = Vec3(-1, top, 0); v.texCoord = Vec2(0, v0); vertices.push_back(v);
        v.position = Vec3( 1, top, 0); v.texCoord = Vec2(1, v0); vertices.push_back(v);
        v.position = Vec3(-1, bottom, 0); v.texCoord = Vec2(0, v1); vertices.push_back(v);
        v.position = Vec3( 1, bottom, 0); v.texCoord = Vec2(1, v1); vertices.push_back(v);
    }
}

void ZigzagPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    for (WORD y = 0; y < m_zigzagRows; ++y)
    {
        WORD base = y * 4;
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 1); indices.push_back(base + 2); indices.push_back(base + 3);
    }
}

} // namespace DX
} // namespace HMREngine
