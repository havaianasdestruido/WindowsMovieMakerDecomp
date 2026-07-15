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
// Simple patterns: BowTie, DiagonalCross, DiagonalBox, Split, Reveal
// ============================================================================
#define SIMPLE_PATTERN_IMPLEMENT(ClassName, TransType, TransName) \
    void ClassName::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) { \
        (void)width; (void)height; \
        float u = (float)width, v = (float)height; \
        PatternVertex v0, v1, v2, v3; \
        v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1); \
        v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1); \
        v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0); \
        v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0); \
        float mask = ApplySoftEdge(m_progress); \
        v0.mask = v1.mask = v2.mask = v3.mask = mask; \
        v0.feather = v1.feather = v2.feather = v3.feather = 0; \
        v0.padding = v1.padding = v2.padding = v3.padding = 0; \
        vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3); \
    } \
    void ClassName::BuildIndices(std::vector<WORD>& indices) { \
        indices.push_back(0); indices.push_back(2); indices.push_back(1); \
        indices.push_back(1); indices.push_back(2); indices.push_back(3); \
    }

SIMPLE_PATTERN_IMPLEMENT(BowTiePatternMesh, BowTie, "BowTie")
SIMPLE_PATTERN_IMPLEMENT(DiagonalCrossPatternMesh, DiagonalCross, "DiagonalCross")
SIMPLE_PATTERN_IMPLEMENT(DiagonalBoxPatternMesh, DiagonalBox, "DiagonalBox")
SIMPLE_PATTERN_IMPLEMENT(SplitPatternMesh, Split, "Split")
SIMPLE_PATTERN_IMPLEMENT(RevealPatternMesh, Reveal, "Reveal")

// ============================================================================
// CirclesPatternMesh
// ============================================================================
void CirclesPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void CirclesPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// CirclePatternMesh
// ============================================================================
void CirclePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void CirclePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// FanAndSweepPatternMesh
// ============================================================================
void FanAndSweepPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void FanAndSweepPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// FanAndSweepBasePatternMesh
// ============================================================================
void FanAndSweepBasePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void FanAndSweepBasePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// FanInPatternMesh
// ============================================================================
void FanInPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void FanInPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// EyePatternMesh
// ============================================================================
void EyePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void EyePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// FanUpPatternMesh
// ============================================================================
void FanUpPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void FanUpPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// FanOutPatternMesh
// ============================================================================
void FanOutPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void FanOutPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// HeartPatternMesh
// ============================================================================
void HeartPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void HeartPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// FillVPatternMesh
// ============================================================================
void FillVPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void FillVPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// KeyholePatternMesh
// ============================================================================
void KeyholePatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void KeyholePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// IrisPatternMesh
// ============================================================================
void IrisPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void IrisPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
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
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void RectanglePatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// StarsPatternMesh
// ============================================================================
void StarsPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void StarsPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// StarPatternMesh
// ============================================================================
void StarPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void StarPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// SweepInPatternMesh
// ============================================================================
void SweepInPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void SweepInPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
}

// ============================================================================
// SweepOutPatternMesh
// ============================================================================
void SweepOutPatternMesh::BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height)
{
    (void)width; (void)height;
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void SweepOutPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
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

        PatternVertex v;
        v.position = Vec3(x, y, 0);
        v.texCoord = Vec2(x * 0.5f + 0.5f, y * 0.5f + 0.5f);
        v.mask = ApplySoftEdge(m_progress);
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
    PatternVertex v0, v1, v2, v3;
    v0.position = Vec3(-1, -1, 0); v0.texCoord = Vec2(0, 1);
    v1.position = Vec3( 1, -1, 0); v1.texCoord = Vec2(1, 1);
    v2.position = Vec3(-1,  1, 0); v2.texCoord = Vec2(0, 0);
    v3.position = Vec3( 1,  1, 0); v3.texCoord = Vec2(1, 0);
    float mask = ApplySoftEdge(m_progress);
    v0.mask = v1.mask = v2.mask = v3.mask = mask;
    v0.feather = v1.feather = v2.feather = v3.feather = 0;
    v0.padding = v1.padding = v2.padding = v3.padding = 0;
    vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2); vertices.push_back(v3);
}

void SweepUpPatternMesh::BuildIndices(std::vector<WORD>& indices)
{
    indices.push_back(0); indices.push_back(2); indices.push_back(1);
    indices.push_back(1); indices.push_back(2); indices.push_back(3);
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
