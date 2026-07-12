// MeshResourceDX.cpp - Vertex/index buffer management implementation

#include "MeshResourceDX.h"
#include <algorithm>
#include <unordered_map>

namespace HMREngine
{
namespace DX
{

// ============================================================================
// MeshResourceDX
// ============================================================================
MeshResourceDX::MeshResourceDX() = default;
MeshResourceDX::~MeshResourceDX() { Release(); }

HRESULT MeshResourceDX::CreateVertexBuffer(const void* vertices, UINT vertexCount,
    UINT vertexStride, bool dynamic)
{
    if (!vertices || !m_device) return E_INVALIDARG;

    m_vertexBuffer.Release();

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = vertexCount * vertexStride;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = m_device->CreateBuffer(&bd, &initData, &m_vertexBuffer);
    if (SUCCEEDED(hr))
    {
        m_vertexCount = vertexCount;
        m_vertexStride = vertexStride;
    }
    return hr;
}

HRESULT MeshResourceDX::CreateIndexBuffer(const void* indices, UINT indexCount,
    DXGI_FORMAT indexFormat)
{
    if (!indices || !m_device) return E_INVALIDARG;

    m_indexBuffer.Release();

    UINT indexSize = (indexFormat == DXGI_FORMAT_R32_UINT) ? 4 : 2;

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = indexCount * indexSize;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;

    HRESULT hr = m_device->CreateBuffer(&bd, &initData, &m_indexBuffer);
    if (SUCCEEDED(hr))
    {
        m_indexCount = indexCount;
        m_indexFormat = indexFormat;
    }
    return hr;
}

HRESULT MeshResourceDX::CreateFromData(const void* vertices, UINT vertexCount, UINT vertexStride,
    const void* indices, UINT indexCount, DXGI_FORMAT indexFormat)
{
    HRESULT hr = CreateVertexBuffer(vertices, vertexCount, vertexStride);
    if (FAILED(hr)) return hr;

    if (indices && indexCount > 0)
    {
        hr = CreateIndexBuffer(indices, indexCount, indexFormat);
        if (FAILED(hr)) return hr;
    }

    return S_OK;
}

void MeshResourceDX::Release()
{
    m_indexBuffer.Release();
    m_vertexBuffer.Release();
    m_vertexCount = 0;
    m_indexCount = 0;
    m_vertexStride = 0;
}

HRESULT MeshResourceDX::MapVertices(void** ppData, UINT* pSize)
{
    if (!m_vertexBuffer || !ppData) return E_FAIL;

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        *ppData = mapped.pData;
        if (pSize) *pSize = m_vertexCount * m_vertexStride;
    }
    return hr;
}

void MeshResourceDX::UnmapVertices()
{
    if (m_vertexBuffer && m_context)
        m_context->Unmap(m_vertexBuffer, 0);
}

HRESULT MeshResourceDX::MapIndices(void** ppData, UINT* pSize)
{
    if (!m_indexBuffer || !ppData) return E_FAIL;

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_context->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        *ppData = mapped.pData;
        if (pSize)
        {
            UINT indexSize = (m_indexFormat == DXGI_FORMAT_R32_UINT) ? 4 : 2;
            *pSize = m_indexCount * indexSize;
        }
    }
    return hr;
}

void MeshResourceDX::UnmapIndices()
{
    if (m_indexBuffer && m_context)
        m_context->Unmap(m_indexBuffer, 0);
}

void MeshResourceDX::Draw(ID3D11DeviceContext* ctx) const
{
    if (!ctx || !m_vertexBuffer) return;

    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &m_vertexBuffer.p, &m_vertexStride, &offset);
    ctx->Draw(m_vertexCount, 0);
}

void MeshResourceDX::DrawIndexed(ID3D11DeviceContext* ctx) const
{
    if (!ctx || !m_vertexBuffer || !m_indexBuffer) return;

    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &m_vertexBuffer.p, &m_vertexStride, &offset);
    ctx->IASetIndexBuffer(m_indexBuffer, m_indexFormat, 0);
    ctx->DrawIndexed(m_indexCount, 0, 0);
}

// ============================================================================
// ComposedGeometryResourceDX
// ============================================================================
ComposedGeometryResourceDX::ComposedGeometryResourceDX() = default;
ComposedGeometryResourceDX::~ComposedGeometryResourceDX() { ClearParts(); }

HRESULT ComposedGeometryResourceDX::AddPart(MeshResourceDX* mesh)
{
    if (!mesh || !m_device) return E_INVALIDARG;

    GeometryPart part;
    part.vertexCount = mesh->GetVertexCount();
    part.indexCount = mesh->GetIndexCount();
    part.vertexStride = mesh->GetVertexStride();
    part.indexFormat = mesh->GetIndexFormat();

    ID3D11Buffer* vb = mesh->GetVertexBuffer();
    if (vb) vb->AddRef();
    part.vertexBuffer.Attach(vb);

    ID3D11Buffer* ib = mesh->GetIndexBuffer();
    if (ib) ib->AddRef();
    part.indexBuffer.Attach(ib);

    m_parts.push_back(std::move(part));
    return S_OK;
}

HRESULT ComposedGeometryResourceDX::AddPart(const void* vertices, UINT vertexCount,
    UINT vertexStride, const void* indices, UINT indexCount)
{
    if (!m_device) return E_FAIL;

    GeometryPart part;
    part.vertexCount = vertexCount;
    part.vertexStride = vertexStride;
    part.indexCount = indexCount;

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = vertexCount * vertexStride;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = m_device->CreateBuffer(&bd, &initData, &part.vertexBuffer);
    if (FAILED(hr)) return hr;

    if (indices && indexCount > 0)
    {
        UINT indexSize = 2;
        bd.ByteWidth = indexCount * indexSize;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

        initData.pSysMem = indices;
        hr = m_device->CreateBuffer(&bd, &initData, &part.indexBuffer);
        if (FAILED(hr)) return hr;
    }

    m_parts.push_back(std::move(part));
    return S_OK;
}

void ComposedGeometryResourceDX::ClearParts()
{
    m_parts.clear();
}

void ComposedGeometryResourceDX::DrawAll(ID3D11DeviceContext* ctx) const
{
    for (UINT i = 0; i < static_cast<UINT>(m_parts.size()); ++i)
        DrawPart(ctx, i);
}

void ComposedGeometryResourceDX::DrawPart(ID3D11DeviceContext* ctx, UINT partIndex) const
{
    if (!ctx || partIndex >= m_parts.size()) return;

    const auto& part = m_parts[partIndex];
    if (!part.vertexBuffer) return;

    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &part.vertexBuffer.p, &part.vertexStride, &offset);

    if (part.indexBuffer && part.indexCount > 0)
    {
        ctx->IASetIndexBuffer(part.indexBuffer, part.indexFormat, 0);
        ctx->DrawIndexed(part.indexCount, 0, 0);
    }
    else
    {
        ctx->Draw(part.vertexCount, 0);
    }
}

UINT ComposedGeometryResourceDX::GetTotalVertexCount() const
{
    UINT total = 0;
    for (const auto& p : m_parts) total += p.vertexCount;
    return total;
}

UINT ComposedGeometryResourceDX::GetTotalIndexCount() const
{
    UINT total = 0;
    for (const auto& p : m_parts) total += p.indexCount;
    return total;
}

// ============================================================================
// GridResourceDX
// ============================================================================
GridResourceDX::GridResourceDX() = default;
GridResourceDX::~GridResourceDX() { Release(); }

HRESULT GridResourceDX::CreateGrid(UINT divisionsX, UINT divisionsY,
    float width, float height)
{
    if (divisionsX == 0 || divisionsY == 0) return E_INVALIDARG;

    Release();

    m_divisionsX = divisionsX;
    m_divisionsY = divisionsY;

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float cellW = width / divisionsX;
    float cellH = height / divisionsY;

    std::vector<VertexPosTex> vertices;
    std::vector<WORD> indices;

    for (UINT y = 0; y <= divisionsY; ++y)
    {
        for (UINT x = 0; x <= divisionsX; ++x)
        {
            VertexPosTex v;
            v.position.x = -halfW + x * cellW;
            v.position.y = -halfH + y * cellH;
            v.position.z = 0.0f;
            v.texCoord.x = (float)x / divisionsX;
            v.texCoord.y = (float)y / divisionsY;
            vertices.push_back(v);
        }
    }

    for (UINT y = 0; y < divisionsY; ++y)
    {
        for (UINT x = 0; x < divisionsX; ++x)
        {
            WORD tl = static_cast<WORD>(y * (divisionsX + 1) + x);
            WORD tr = tl + 1;
            WORD bl = static_cast<WORD>((y + 1) * (divisionsX + 1) + x);
            WORD br = bl + 1;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);
            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }
    }

    return CreateFromData(vertices.data(), static_cast<UINT>(vertices.size()),
        sizeof(VertexPosTex), indices.data(), static_cast<UINT>(indices.size()));
}

HRESULT GridResourceDX::CreateGridFullscreen()
{
    return CreateGrid(1, 1, 2.0f, 2.0f);
}

// ============================================================================
// ShatterGridResourceDX
// ============================================================================
ShatterGridResourceDX::ShatterGridResourceDX() = default;
ShatterGridResourceDX::~ShatterGridResourceDX() { Release(); }

HRESULT ShatterGridResourceDX::CreateShatterGrid(UINT piecesX, UINT piecesY,
    float width, float height, float spread, float rotation)
{
    if (piecesX == 0 || piecesY == 0) return E_INVALIDARG;

    Release();

    m_piecesX = piecesX;
    m_piecesY = piecesY;

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float pieceW = width / piecesX;
    float pieceH = height / piecesY;

    m_pieceTransforms.resize(piecesX * piecesY);
    for (auto& pt : m_pieceTransforms) { pt.offset = Vec3(); pt.rotation = 0.0f; }

    std::vector<ShatterVertex> vertices;
    std::vector<WORD> indices;

    for (UINT y = 0; y < piecesY; ++y)
    {
        for (UINT x = 0; x < piecesX; ++x)
        {
            float left = -halfW + x * pieceW;
            float top = -halfH + y * pieceH;
            float right = left + pieceW;
            float bottom = top + pieceH;

            float u0 = (float)x / piecesX;
            float v0 = (float)y / piecesY;
            float u1 = (float)(x + 1) / piecesX;
            float v1 = (float)(y + 1) / piecesY;

            float cx = (left + right) * 0.5f;
            float cy = (top + bottom) * 0.5f;

            WORD base = static_cast<WORD>(vertices.size());

            ShatterVertex v;
            v.pieceIndex = (float)(y * piecesX + x);

            v.position = Vec3(left, top, 0.0f); v.texCoord = Vec2(u0, v0); v.pieceCenter = Vec2(cx, cy);
            vertices.push_back(v);

            v.position = Vec3(right, top, 0.0f); v.texCoord = Vec2(u1, v0); v.pieceCenter = Vec2(cx, cy);
            vertices.push_back(v);

            v.position = Vec3(left, bottom, 0.0f); v.texCoord = Vec2(u0, v1); v.pieceCenter = Vec2(cx, cy);
            vertices.push_back(v);

            v.position = Vec3(right, bottom, 0.0f); v.texCoord = Vec2(u1, v1); v.pieceCenter = Vec2(cx, cy);
            vertices.push_back(v);

            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 1);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }
    }

    return CreateFromData(vertices.data(), static_cast<UINT>(vertices.size()),
        sizeof(ShatterVertex), indices.data(), static_cast<UINT>(indices.size()));
}

void ShatterGridResourceDX::SetPieceOffset(UINT pieceIndex, const Vec3& offset)
{
    if (pieceIndex < m_pieceTransforms.size())
        m_pieceTransforms[pieceIndex].offset = offset;
}

void ShatterGridResourceDX::SetPieceRotation(UINT pieceIndex, float angle)
{
    if (pieceIndex < m_pieceTransforms.size())
        m_pieceTransforms[pieceIndex].rotation = angle;
}

void ShatterGridResourceDX::ResetTransforms()
{
    for (auto& pt : m_pieceTransforms) { pt.offset = Vec3(); pt.rotation = 0.0f; }
}

// ============================================================================
// PageCurlGridResourceDX
// ============================================================================
PageCurlGridResourceDX::PageCurlGridResourceDX() = default;
PageCurlGridResourceDX::~PageCurlGridResourceDX() { Release(); }

HRESULT PageCurlGridResourceDX::CreatePageCurlGrid(UINT segmentsX, UINT segmentsY,
    float width, float height, float curlRadius)
{
    if (segmentsX == 0 || segmentsY == 0) return E_INVALIDARG;

    Release();

    m_segmentsX = segmentsX;
    m_segmentsY = segmentsY;
    m_curlRadius = curlRadius;

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float segW = width / segmentsX;
    float segH = height / segmentsY;

    std::vector<PageCurlVertex> vertices;
    std::vector<WORD> indices;

    for (UINT y = 0; y <= segmentsY; ++y)
    {
        for (UINT x = 0; x <= segmentsX; ++x)
        {
            PageCurlVertex v;
            float px = -halfW + x * segW;
            float py = -halfH + y * segH;
            v.position = Vec3(px, py, 0.0f);
            v.texCoord = Vec2((float)x / segmentsX, (float)y / segmentsY);
            v.normal = Vec3(0.0f, 0.0f, 1.0f);
            v.curlFactor = (float)x / segmentsX;
            vertices.push_back(v);
        }
    }

    for (UINT y = 0; y < segmentsY; ++y)
    {
        for (UINT x = 0; x < segmentsX; ++x)
        {
            WORD tl = static_cast<WORD>(y * (segmentsX + 1) + x);
            WORD tr = tl + 1;
            WORD bl = static_cast<WORD>((y + 1) * (segmentsX + 1) + x);
            WORD br = bl + 1;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);
            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }
    }

    return CreateFromData(vertices.data(), static_cast<UINT>(vertices.size()),
        sizeof(PageCurlVertex), indices.data(), static_cast<UINT>(indices.size()));
}

void PageCurlGridResourceDX::SetCurlProgress(float progress) { m_curlProgress = Saturate(progress); }
void PageCurlGridResourceDX::SetCurlDirection(float direction) { m_curlDirection = direction; }

// ============================================================================
// ScrollingTextResourceDX
// ============================================================================
ScrollingTextResourceDX::ScrollingTextResourceDX() = default;
ScrollingTextResourceDX::~ScrollingTextResourceDX() { Release(); }

HRESULT ScrollingTextResourceDX::CreateScrollingTextMesh(UINT width, UINT height)
{
    Release();

    m_meshWidth = width;
    m_meshHeight = height;

    float w = 2.0f;
    float h = 2.0f;

    VertexPosTex vertices[] =
    {
        { Vec3(-w * 0.5f, -h * 0.5f, 0.0f), Vec2(0.0f, 1.0f) },
        { Vec3(-w * 0.5f,  h * 0.5f, 0.0f), Vec2(0.0f, 0.0f) },
        { Vec3( w * 0.5f,  h * 0.5f, 0.0f), Vec2(1.0f, 0.0f) },
        { Vec3( w * 0.5f, -h * 0.5f, 0.0f), Vec2(1.0f, 1.0f) },
    };

    WORD indices[] = { 0, 1, 2, 0, 2, 3 };

    return CreateFromData(vertices, 4, sizeof(VertexPosTex), indices, 6);
}

// ============================================================================
// WipeMeshResourceDX
// ============================================================================
WipeMeshResourceDX::WipeMeshResourceDX() = default;
WipeMeshResourceDX::~WipeMeshResourceDX() { Release(); }

HRESULT WipeMeshResourceDX::CreateWipeMesh(UINT segmentsX, UINT segmentsY,
    float width, float height)
{
    if (segmentsX == 0 || segmentsY == 0) return E_INVALIDARG;

    Release();

    m_segmentsX = segmentsX;
    m_segmentsY = segmentsY;

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float segW = width / segmentsX;
    float segH = height / segmentsY;

    std::vector<VertexPosTex> vertices;
    std::vector<WORD> indices;

    for (UINT y = 0; y <= segmentsY; ++y)
    {
        for (UINT x = 0; x <= segmentsX; ++x)
        {
            VertexPosTex v;
            v.position.x = -halfW + x * segW;
            v.position.y = -halfH + y * segH;
            v.position.z = 0.0f;
            v.texCoord.x = (float)x / segmentsX;
            v.texCoord.y = (float)y / segmentsY;
            vertices.push_back(v);
        }
    }

    for (UINT y = 0; y < segmentsY; ++y)
    {
        for (UINT x = 0; x < segmentsX; ++x)
        {
            WORD tl = static_cast<WORD>(y * (segmentsX + 1) + x);
            WORD tr = tl + 1;
            WORD bl = static_cast<WORD>((y + 1) * (segmentsX + 1) + x);
            WORD br = bl + 1;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);
            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }
    }

    return CreateFromData(vertices.data(), static_cast<UINT>(vertices.size()),
        sizeof(VertexPosTex), indices.data(), static_cast<UINT>(indices.size()));
}

void WipeMeshResourceDX::SetWipeProgress(float progress) { m_wipeProgress = Saturate(progress); }
void WipeMeshResourceDX::SetWipeDirection(float direction) { m_wipeDirection = direction; }

// ============================================================================
// PreprocessMesh
// ============================================================================
HRESULT PreprocessMesh::ComputeNormals(
    const VertexPos* vertices, UINT vertexCount,
    const WORD* indices, UINT indexCount,
    Vec3* outNormals)
{
    if (!vertices || !outNormals || vertexCount == 0) return E_INVALIDARG;

    memset(outNormals, 0, sizeof(Vec3) * vertexCount);

    for (UINT i = 0; i + 2 < indexCount; i += 3)
    {
        const Vec3& p0 = vertices[indices[i]].position;
        const Vec3& p1 = vertices[indices[i + 1]].position;
        const Vec3& p2 = vertices[indices[i + 2]].position;

        Vec3 edge1 = p1 - p0;
        Vec3 edge2 = p2 - p0;
        Vec3 n = edge1.Cross(edge2);

        outNormals[indices[i]] = outNormals[indices[i]] + n;
        outNormals[indices[i + 1]] = outNormals[indices[i + 1]] + n;
        outNormals[indices[i + 2]] = outNormals[indices[i + 2]] + n;
    }

    for (UINT i = 0; i < vertexCount; ++i)
    {
        outNormals[i] = outNormals[i].Normalized();
    }

    return S_OK;
}

HRESULT PreprocessMesh::ComputeTangents(
    const VertexPosNormTex* vertices, UINT vertexCount,
    const WORD* indices, UINT indexCount,
    Vec3* outTangents, Vec3* outBiTangents)
{
    if (!vertices || !outTangents || !outBiTangents) return E_INVALIDARG;

    memset(outTangents, 0, sizeof(Vec3) * vertexCount);
    memset(outBiTangents, 0, sizeof(Vec3) * vertexCount);

    for (UINT i = 0; i + 2 < indexCount; i += 3)
    {
        WORD i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];

        const Vec3& p0 = vertices[i0].position;
        const Vec3& p1 = vertices[i1].position;
        const Vec3& p2 = vertices[i2].position;

        const Vec2& uv0 = vertices[i0].texCoord;
        const Vec2& uv1 = vertices[i1].texCoord;
        const Vec2& uv2 = vertices[i2].texCoord;

        Vec3 edge1 = p1 - p0;
        Vec3 edge2 = p2 - p0;
        Vec2 duv1 = uv1 - uv0;
        Vec2 duv2 = uv2 - uv0;

        float det = duv1.x * duv2.y - duv1.y * duv2.x;
        if (fabsf(det) < 1e-6f) continue;

        float invDet = 1.0f / det;
        Vec3 t = (edge1 * duv2.y - edge2 * duv1.y) * invDet;
        Vec3 b = (edge2 * duv1.x - edge1 * duv2.x) * invDet;

        outTangents[i0] = outTangents[i0] + t;
        outTangents[i1] = outTangents[i1] + t;
        outTangents[i2] = outTangents[i2] + t;

        outBiTangents[i0] = outBiTangents[i0] + b;
        outBiTangents[i1] = outBiTangents[i1] + b;
        outBiTangents[i2] = outBiTangents[i2] + b;
    }

    for (UINT i = 0; i < vertexCount; ++i)
    {
        outTangents[i] = outTangents[i].Normalized();
        outBiTangents[i] = outBiTangents[i].Normalized();
    }

    return S_OK;
}

HRESULT PreprocessMesh::ComputeBoundingSphere(
    const VertexPos* vertices, UINT vertexCount,
    Vec3& outCenter, float& outRadius)
{
    if (!vertices || vertexCount == 0) return E_INVALIDARG;

    Vec3 minPt(FLT_MAX, FLT_MAX, FLT_MAX);
    Vec3 maxPt(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (UINT i = 0; i < vertexCount; ++i)
    {
        const Vec3& p = vertices[i].position;
        minPt.x = (std::min)(minPt.x, p.x);
        minPt.y = (std::min)(minPt.y, p.y);
        minPt.z = (std::min)(minPt.z, p.z);
        maxPt.x = (std::max)(maxPt.x, p.x);
        maxPt.y = (std::max)(maxPt.y, p.y);
        maxPt.z = (std::max)(maxPt.z, p.z);
    }

    outCenter = (minPt + maxPt) * 0.5f;
    outRadius = 0.0f;

    for (UINT i = 0; i < vertexCount; ++i)
    {
        float d = (vertices[i].position - outCenter).Length();
        if (d > outRadius) outRadius = d;
    }

    return S_OK;
}

HRESULT PreprocessMesh::ComputeBoundingBox(
    const VertexPos* vertices, UINT vertexCount,
    Vec3& outMin, Vec3& outMax)
{
    if (!vertices || vertexCount == 0) return E_INVALIDARG;

    outMin = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    outMax = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (UINT i = 0; i < vertexCount; ++i)
    {
        const Vec3& p = vertices[i].position;
        outMin.x = (std::min)(outMin.x, p.x);
        outMin.y = (std::min)(outMin.y, p.y);
        outMin.z = (std::min)(outMin.z, p.z);
        outMax.x = (std::max)(outMax.x, p.x);
        outMax.y = (std::max)(outMax.y, p.y);
        outMax.z = (std::max)(outMax.z, p.z);
    }

    return S_OK;
}

HRESULT PreprocessMesh::WeldVertices(
    std::vector<VertexPosNormTex>& vertices,
    std::vector<WORD>& indices,
    float tolerance)
{
    if (vertices.empty()) return S_OK;

    float tolSq = tolerance * tolerance;
    std::vector<WORD> remap(vertices.size());
    std::vector<VertexPosNormTex> unique;

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        bool found = false;
        for (size_t j = 0; j < unique.size(); ++j)
        {
            Vec3 diff = vertices[i].position - unique[j].position;
            if (diff.LengthSq() < tolSq)
            {
                remap[i] = static_cast<WORD>(j);
                found = true;
                break;
            }
        }

        if (!found)
        {
            remap[i] = static_cast<WORD>(unique.size());
            unique.push_back(vertices[i]);
        }
    }

    for (auto& idx : indices)
        idx = remap[idx];

    vertices = std::move(unique);
    return S_OK;
}

HRESULT PreprocessMesh::OptimizeIndices(
    std::vector<WORD>& indices,
    UINT vertexCount)
{
    (void)vertexCount;
    return S_OK;
}

} // namespace DX
} // namespace HMREngine
