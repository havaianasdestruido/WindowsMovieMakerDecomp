#pragma once
// IndexedFaceSetImpl.h - IndexedFaceSet and vertex attribute nodes

#include "HMREngine.h"
#include "X3DNodes.h"
#include "CoordinateImpl.h"
#include <d3d11.h>
#include <atlbase.h>
#include <vector>

namespace HMREngine
{
    struct VertexAttributes
    {
        Vec3 position;
        Vec3 normal;
        Vec2 texCoord;
        Rgb color;
    };

    class IndexedFaceSetImpl : public ImplObject<IndexedFaceSetImpl>
    {
    public:
        IndexedFaceSetImpl();
        virtual ~IndexedFaceSetImpl();

        HRESULT Initialize(IndexedFaceSetNode* node, ID3D11Device* dev);
        void Shutdown();

        HRESULT RebuildBuffers(ID3D11Device* dev);

        ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer; }
        ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer; }
        UINT GetVertexCount() const { return m_vertexCount; }
        UINT GetIndexCount() const { return m_indexCount; }
        UINT GetVertexStride() const { return sizeof(VertexAttributes); }

        bool IsCCW() const { return m_ccw; }
        bool IsSolid() const { return m_solid; }

        // Bounding info
        Vec3 GetBoundingBoxMin() const { return m_bboxMin; }
        Vec3 GetBoundingBoxMax() const { return m_bboxMax; }
        void ComputeBoundingBox();

    private:
        IndexedFaceSetNode* m_node = nullptr;

        CComPtr<ID3D11Buffer> m_vertexBuffer;
        CComPtr<ID3D11Buffer> m_indexBuffer;
        UINT m_vertexCount = 0;
        UINT m_indexCount = 0;

        bool m_ccw = true;
        bool m_solid = true;

        Vec3 m_bboxMin;
        Vec3 m_bboxMax;

        std::vector<VertexAttributes> m_vertices;
        std::vector<UINT> m_indices;

        void TriangulateFace(const std::vector<int>& faceIndices, std::vector<UINT>& outIndices);
        Vec3 ComputeFaceNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2);
    };

} // namespace HMREngine
