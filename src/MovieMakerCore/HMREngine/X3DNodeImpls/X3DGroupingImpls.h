#pragma once
// X3DGroupingImpls.h - Grouping, shape, geometry, child, bounded node bridges

#include "X3DNodeImplBase.h"

namespace HMREngine
{
    // --- X3DGroupingNodeImpl: Grouping node bridge ---
    class X3DGroupingNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DGroupingNodeImpl();
        virtual ~X3DGroupingNodeImpl();

        HRESULT Initialize(X3DGroupingNode* node);
        void Shutdown() override;

        X3DGroupingNode* GetGroupingNode() const { return m_groupingNode; }

        void AddGroupChild(X3DChildNodeImpl* child);
        void RemoveGroupChild(X3DChildNodeImpl* child);
        UINT GetGroupChildCount() const;
        X3DChildNodeImpl* GetGroupChild(UINT index) const;

        void SetBBoxCenter(const Vec3& center) { m_bboxCenter = center; }
        void SetBBoxSize(const Vec3& size) { m_bboxSize = size; }

    protected:
        X3DGroupingNode* m_groupingNode = nullptr;
        Vec3 m_bboxCenter;
        Vec3 m_bboxSize = Vec3(-1, -1, -1);

        HRESULT SetupFields() override;
    };

    // --- X3DShapeNodeImpl: Shape node bridge ---
    class X3DShapeNodeImpl : public X3DChildNodeImpl, public TransparentObject
    {
    public:
        X3DShapeNodeImpl();
        virtual ~X3DShapeNodeImpl();

        HRESULT Initialize(X3DShapeNode* node);
        void Shutdown() override;

        X3DShapeNode* GetShapeNode() const { return m_shapeNode; }

        void SetGeometry(X3DGeometryNodeImpl* geo);
        void SetAppearance(X3DAppearanceNodeImpl* app);

        X3DGeometryNodeImpl* GetGeometry() const { return m_geometry; }
        X3DAppearanceNodeImpl* GetAppearance() const { return m_appearance; }

        void SetCastShadow(bool shadow) { m_castShadow = shadow; }
        bool GetCastShadow() const { return m_castShadow; }

    protected:
        X3DShapeNode* m_shapeNode = nullptr;
        X3DGeometryNodeImpl* m_geometry = nullptr;
        X3DAppearanceNodeImpl* m_appearance = nullptr;
        bool m_castShadow = true;

        HRESULT SetupFields() override;
    };

    // --- X3DGeometryNodeImpl: Geometry node bridge ---
    class X3DGeometryNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DGeometryNodeImpl();
        virtual ~X3DGeometryNodeImpl();

        HRESULT Initialize(X3DGeometryNode* node);
        void Shutdown() override;

        X3DGeometryNode* GetGeometryNode() const { return m_geometryNode; }

        virtual HRESULT BuildVertexBuffer(ID3D11Device* dev, ID3D11Buffer** ppVB);
        virtual HRESULT BuildIndexBuffer(ID3D11Device* dev, ID3D11Buffer** ppIB);
        virtual UINT GetVertexCount() const;
        virtual UINT GetIndexCount() const;

        bool IsSolid() const { return m_solid; }
        void SetSolid(bool solid) { m_solid = solid; }

    protected:
        X3DGeometryNode* m_geometryNode = nullptr;
        bool m_solid = true;

        HRESULT SetupFields() override;
    };

    // --- X3DComposedGeometryNodeImpl: Composed geometry bridge ---
    class X3DComposedGeometryNodeImpl : public X3DGeometryNodeImpl
    {
    public:
        X3DComposedGeometryNodeImpl();
        virtual ~X3DComposedGeometryNodeImpl();

        HRESULT Initialize(X3DGeometryNode* node);
        void Shutdown() override;

        HRESULT BuildVertexBuffer(ID3D11Device* dev, ID3D11Buffer** ppVB) override;
        HRESULT BuildIndexBuffer(ID3D11Device* dev, ID3D11Buffer** ppIB) override;
        UINT GetVertexCount() const override;
        UINT GetIndexCount() const override;

        void SetCoordIndex(const MFInt32& indices) { m_coordIndex = indices; }
        void SetNormalIndex(const MFInt32& indices) { m_normalIndex = indices; }
        void SetTexCoordIndex(const MFInt32& indices) { m_texCoordIndex = indices; }

        void SetCCW(bool ccw) { m_ccw = ccw; }
        void SetConvex(bool convex) { m_convex = convex; }

    protected:
        MFInt32 m_coordIndex;
        MFInt32 m_normalIndex;
        MFInt32 m_texCoordIndex;
        bool m_ccw = true;
        bool m_convex = true;

        HRESULT SetupFields() override;
    };

} // namespace HMREngine
