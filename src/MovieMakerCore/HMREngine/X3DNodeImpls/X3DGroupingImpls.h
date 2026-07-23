#pragma once
// X3DGroupingImpls.h - Grouping, shape, geometry, child, bounded node bridges

#include "X3DNodeImplBase.h"

namespace HMREngine
{
    // Forward declarations for types used before definition
    class X3DGeometryNodeImpl;
    class X3DAppearanceNodeImpl;
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

    // --- TransformNodeImpl: Transform node bridge ---
    class TransformNodeImpl : public X3DGroupingNodeImpl
    {
    public:
        TransformNodeImpl();
        virtual ~TransformNodeImpl();

        HRESULT Initialize(TransformNode* node);
        void Shutdown() override;

        TransformNode* GetTransformNode() const { return m_transformNode; }

        void SetTranslation(const Vec3& t);
        void SetRotation(const Rotation4f& r);
        void SetScale(const Vec3& s);
        void SetCenter(const Vec3& c);

        const Vec3& GetTranslation() const { return m_translation; }
        const Rotation4f& GetRotation() const { return m_rotation; }
        const Vec3& GetScale() const { return m_scale; }
        const Vec3& GetCenter() const { return m_center; }

        Matrix4f GetLocalMatrix() const;
        Matrix4f GetWorldMatrix() const;
        void RebuildMatrix();

    protected:
        TransformNode* m_transformNode = nullptr;
        Vec3 m_translation;
        Rotation4f m_rotation;
        Vec3 m_scale = Vec3(1, 1, 1);
        Vec3 m_center;
        Matrix4f m_worldMatrix;
        bool m_matrixDirty = true;

        HRESULT SetupFields() override;
    };

    // --- GroupNodeImpl: Group node bridge ---
    class GroupNodeImpl : public X3DGroupingNodeImpl
    {
    public:
        GroupNodeImpl();
        virtual ~GroupNodeImpl();

        HRESULT Initialize(GroupNode* node);
        void Shutdown() override;

        GroupNode* GetGroupNode() const { return m_groupNode; }

        void Traverse(std::function<void(X3DChildNodeImpl*, int)> visitor, int depth = 0) override;

    protected:
        GroupNode* m_groupNode = nullptr;

        HRESULT SetupFields() override;
    };

    // --- SwitchNodeImpl: Switch node bridge ---
    class SwitchNodeImpl : public X3DGroupingNodeImpl
    {
    public:
        SwitchNodeImpl();
        virtual ~SwitchNodeImpl();

        HRESULT Initialize(SwitchNode* node);
        void Shutdown() override;

        SwitchNode* GetSwitchNode() const { return m_switchNode; }

        int GetChoice() const { return m_choice; }
        void SetChoice(int choice);

        bool IsSwitchEnabled() const { return m_switchEnabled; }
        void SetSwitchEnabled(bool enabled) { m_switchEnabled = enabled; MarkDirty(); }

        X3DChildNodeImpl* GetActiveChild() const;
        void TraverseActive(std::function<void(X3DChildNodeImpl*, int)> visitor, int depth = 0);

    protected:
        SwitchNode* m_switchNode = nullptr;
        int m_choice = 0;
        bool m_switchEnabled = true;

        HRESULT SetupFields() override;
    };

    // --- BillboardNodeImpl: Billboard node bridge ---
    class BillboardNodeImpl : public X3DGroupingNodeImpl
    {
    public:
        BillboardNodeImpl();
        virtual ~BillboardNodeImpl();

        HRESULT Initialize(X3DGroupingNode* node);
        void Shutdown() override;

        void SetAxisOfRotation(const Vec3& axis) { m_axisOfRotation = axis; m_billboardDirty = true; }
        const Vec3& GetAxisOfRotation() const { return m_axisOfRotation; }

        void UpdateBillboard(const Vec3& eyePosition);
        Matrix4f GetBillboardMatrix() const;

    protected:
        Vec3 m_axisOfRotation = Vec3(0, 1, 0);
        Matrix4f m_billboardMatrix;
        bool m_billboardDirty = true;

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
