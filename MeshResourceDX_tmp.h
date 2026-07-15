#pragma once
// MeshResourceDX.h - Vertex/index buffer management, geometry meshes for transitions

#include "DXResources.h"
#include "DeviceClientDX.h"

namespace HMREngine
{
    namespace DX
    {
        // Vertex formats used throughout the engine
        struct VertexPos
        {
            Vec3 position;
        };

        struct VertexPosTex
        {
            Vec3 position;
            Vec2 texCoord;
        };

        struct VertexPosNormTex
        {
            Vec3 position;
            Vec3 normal;
            Vec2 texCoord;
        };

        struct VertexPosColor
        {
            Vec3 position;
            Rgba color;
        };

        struct VertexPosNormColor
        {
            Vec3 position;
            Vec3 normal;
            Rgba color;
        };

        // --- MeshResourceDX: Vertex/index buffer management ---
        class MeshResourceDX : public DeviceClientDXImpl
        {
        public:
            MeshResourceDX();
            virtual ~MeshResourceDX();

            HRESULT CreateVertexBuffer(const void* vertices, UINT vertexCount,
                UINT vertexStride, bool dynamic = false);
            HRESULT CreateIndexBuffer(const void* indices, UINT indexCount,
                DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT);
            HRESULT CreateFromData(const void* vertices, UINT vertexCount, UINT vertexStride,
                const void* indices, UINT indexCount, DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT);
            void Release();

            HRESULT MapVertices(void** ppData, UINT* pSize);
            void UnmapVertices();
            HRESULT MapIndices(void** ppData, UINT* pSize);
            void UnmapIndices();

            void Draw(ID3D11DeviceContext* ctx) const;
            void DrawIndexed(ID3D11DeviceContext* ctx) const;

            ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer; }
            ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer; }
            UINT GetVertexCount() const { return m_vertexCount; }
            UINT GetIndexCount() const { return m_indexCount; }
            UINT GetVertexStride() const { return m_vertexStride; }
            DXGI_FORMAT GetIndexFormat() const { return m_indexFormat; }

        protected:
            CComPtr<ID3D11Buffer> m_vertexBuffer;
            CComPtr<ID3D11Buffer> m_indexBuffer;
            UINT m_vertexCount = 0;
            UINT m_indexCount = 0;
            UINT m_vertexStride = 0;
            DXGI_FORMAT m_indexFormat = DXGI_FORMAT_R16_UINT;
        };

        // --- ComposedGeometryResourceDX: Multi-part mesh ---
        class ComposedGeometryResourceDX : public DeviceClientDXImpl
        {
        public:
            ComposedGeometryResourceDX();
            virtual ~ComposedGeometryResourceDX();

            HRESULT AddPart(MeshResourceDX* mesh);
            HRESULT AddPart(const void* vertices, UINT vertexCount, UINT vertexStride,
                const void* indices, UINT indexCount);
            void ClearParts();
            void Release();

            void DrawAll(ID3D11DeviceContext* ctx) const;
            void DrawPart(ID3D11DeviceContext* ctx, UINT partIndex) const;

            UINT GetPartCount() const { return static_cast<UINT>(m_parts.size()); }
            UINT GetTotalVertexCount() const;
            UINT GetTotalIndexCount() const;

        protected:
            struct GeometryPart
            {
                CComPtr<ID3D11Buffer> vertexBuffer;
                CComPtr<ID3D11Buffer> indexBuffer;
                UINT vertexCount = 0;
                UINT indexCount = 0;
                UINT vertexStride = 0;
                DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
            };

            std::vector<GeometryPart> m_parts;
        };

        // --- GridResourceDX: Grid mesh for transitions ---
        class GridResourceDX : public MeshResourceDX
        {
        public:
            GridResourceDX();
            virtual ~GridResourceDX();

            HRESULT CreateGrid(UINT divisionsX, UINT divisionsY,
                float width = 2.0f, float height = 2.0f);
            HRESULT CreateGridFullscreen();
            void Release();

            UINT GetDivisionsX() const { return m_divisionsX; }
            UINT GetDivisionsY() const { return m_divisionsY; }

        protected:
            UINT m_divisionsX = 1;
            UINT m_divisionsY = 1;
        };

        // --- ShatterGridResourceDX: Shatter transition mesh ---
        class ShatterGridResourceDX : public MeshResourceDX
        {
        public:
            ShatterGridResourceDX();
            virtual ~ShatterGridResourceDX();

            HRESULT CreateShatterGrid(UINT piecesX, UINT piecesY,
                float width = 2.0f, float height = 2.0f,
                float spread = 0.0f, float rotation = 0.0f);
            void Release();

            void SetPieceOffset(UINT pieceIndex, const Vec3& offset);
            void SetPieceRotation(UINT pieceIndex, float angle);
            void ResetTransforms();

            UINT GetPieceCount() const { return m_piecesX * m_piecesY; }

        protected:
            UINT m_piecesX = 4;
            UINT m_piecesY = 4;

            struct PieceTransform
            {
                Vec3 offset;
                float rotation = 0.0f;
            };

            std::vector<PieceTransform> m_pieceTransforms;

            struct ShatterVertex
            {
                Vec3 position;
                Vec2 texCoord;
                Vec2 pieceCenter;
                float pieceIndex;
                float padding;
            };
        };

        // --- PageCurlGridResourceDX: Page curl transition mesh ---
        class PageCurlGridResourceDX : public MeshResourceDX
        {
        public:
            PageCurlGridResourceDX();
            virtual ~PageCurlGridResourceDX();

            HRESULT CreatePageCurlGrid(UINT segmentsX, UINT segmentsY,
                float width = 2.0f, float height = 2.0f,
                float curlRadius = 1.0f);
            void Release();

            void SetCurlProgress(float progress);
            void SetCurlDirection(float direction);

        protected:
            UINT m_segmentsX = 8;
            UINT m_segmentsY = 8;
            float m_curlRadius = 1.0f;
            float m_curlProgress = 0.0f;
            float m_curlDirection = 1.0f;

            struct PageCurlVertex
            {
                Vec3 position;
                Vec2 texCoord;
                Vec3 normal;
                float curlFactor;
            };
        };

        // --- ScrollingTextResourceDX: Mesh for scrolling text ---
        class ScrollingTextResourceDX : public MeshResourceDX
        {
        public:
            ScrollingTextResourceDX();
            virtual ~ScrollingTextResourceDX();

            HRESULT CreateScrollingTextMesh(UINT width, UINT height);
            void Release();

        protected:
            UINT m_meshWidth = 0;
            UINT m_meshHeight = 0;
        };

        // --- WipeMeshResourceDX: Wipe transition mesh ---
        class WipeMeshResourceDX : public MeshResourceDX
        {
        public:
            WipeMeshResourceDX();
            virtual ~WipeMeshResourceDX();

            HRESULT CreateWipeMesh(UINT segmentsX, UINT segmentsY,
                float width = 2.0f, float height = 2.0f);
            void Release();

            void SetWipeProgress(float progress);
            void SetWipeDirection(float direction);

        protected:
            UINT m_segmentsX = 1;
            UINT m_segmentsY = 1;
            float m_wipeProgress = 0.0f;
            float m_wipeDirection = 0.0f;
        };

        // --- PreprocessMesh: Mesh preprocessing utilities ---
        class PreprocessMesh
        {
        public:
            static HRESULT ComputeNormals(
                const VertexPos* vertices, UINT vertexCount,
                const WORD* indices, UINT indexCount,
                Vec3* outNormals);

            static HRESULT ComputeTangents(
                const VertexPosNormTex* vertices, UINT vertexCount,
                const WORD* indices, UINT indexCount,
                Vec3* outTangents, Vec3* outBiTangents);

            static HRESULT ComputeBoundingSphere(
                const VertexPos* vertices, UINT vertexCount,
                Vec3& outCenter, float& outRadius);

            static HRESULT ComputeBoundingBox(
                const VertexPos* vertices, UINT vertexCount,
                Vec3& outMin, Vec3& outMax);

            static HRESULT WeldVertices(
                std::vector<VertexPosNormTex>& vertices,
                std::vector<WORD>& indices,
                float tolerance = 0.0001f);

            static HRESULT OptimizeIndices(
                std::vector<WORD>& indices,
                UINT vertexCount);
        };

    } // namespace DX
} // namespace HMREngine
