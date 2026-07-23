#pragma once
// X3DNodeImplBase.h - Base bridge between X3D node and engine field system

#include "../HMREngine.h"
#include "../X3DNodes.h"
#include "../X3DFieldTypes.h"

namespace HMREngine
{
    // --- FieldImpl: Individual field implementation ---
    class FieldImpl
    {
    public:
        FieldImpl();
        virtual ~FieldImpl();

        HRESULT Initialize(const std::string& name, FieldType type, X3DFieldNode* field);
        void Shutdown();

        const std::string& GetName() const { return m_name; }
        FieldType GetFieldType() const { return m_type; }
        X3DFieldNode* GetField() const { return m_field; }

        HRESULT SetValue(X3DFieldNode* value);
        HRESULT GetValue(X3DFieldNode** ppValue) const;

        bool IsModified() const { return m_modified; }
        void ClearModified() { m_modified = false; }

        using ChangeCallback = std::function<void(FieldImpl*)>;
        void SetChangeCallback(ChangeCallback cb) { m_changeCallback = cb; }

    protected:
        std::string m_name;
        FieldType m_type = FieldType::SFBool;
        X3DFieldNode* m_field = nullptr;
        bool m_modified = false;
        ChangeCallback m_changeCallback;
    };

    // --- FieldCollection: Manages field collections for nodes ---
    class FieldCollection
    {
    public:
        FieldCollection();
        ~FieldCollection();

        HRESULT AddField(const std::string& name, FieldType type, X3DFieldNode* field);
        HRESULT RemoveField(const std::string& name);
        FieldImpl* GetField(const std::string& name) const;
        HRESULT GetFieldByIndex(UINT index, FieldImpl** ppField) const;

        UINT GetFieldCount() const { return static_cast<UINT>(m_fields.size()); }
        bool HasField(const std::string& name) const;

        void Clear();

        using CollectionChangedCallback = std::function<void(const std::string&, bool)>;
        void SetCollectionChangedCallback(CollectionChangedCallback cb) { m_changedCallback = cb; }

    protected:
        std::vector<std::unique_ptr<FieldImpl>> m_fields;
        std::map<std::string, UINT> m_fieldIndex;
        CollectionChangedCallback m_changedCallback;
    };

    // --- IsActiveBase: Active state tracking base ---
    class IsActiveBase
    {
    public:
        IsActiveBase();
        virtual ~IsActiveBase();

        bool IsActive() const { return m_isActive; }
        void SetActive(bool active);

        using ActiveChangedCallback = std::function<void(bool)>;
        void SetActiveChangedCallback(ActiveChangedCallback cb) { m_activeChangedCallback = cb; }

    protected:
        bool m_isActive = false;
        ActiveChangedCallback m_activeChangedCallback;

        virtual void OnActiveChanged(bool active) {}
    };

    // --- TransparentObject: Transparency marker ---
    class TransparentObject
    {
    public:
        TransparentObject();
        virtual ~TransparentObject();

        bool IsTransparent() const { return m_transparent; }
        void SetTransparent(bool transparent) { m_transparent = transparent; }

        float GetTransparency() const { return m_transparency; }
        void SetTransparency(float t) { m_transparency = t; }

    protected:
        bool m_transparent = false;
        float m_transparency = 0.0f;
    };

    // --- X3DNodeImpl: Base bridge between X3D node and engine field system ---
    class X3DNodeImpl
    {
    public:
        X3DNodeImpl();
        virtual ~X3DNodeImpl();

        HRESULT Initialize(X3DNode* node);
        virtual void Shutdown();

        X3DNode* GetNode() const { return m_node; }
        const std::string& GetNodeName() const { return m_nodeName; }
        const std::string& GetNodeTypeName() const { return m_nodeTypeName; }

        FieldCollection* GetFields() { return &m_fields; }
        const FieldCollection* GetFields() const { return &m_fields; }

        virtual HRESULT OnFieldChanged(const std::string& fieldName, X3DFieldNode* value);
        virtual HRESULT OnPropertyChanged(const std::string& propertyName);

        bool IsDirty() const { return m_dirty; }
        void ClearDirty() { m_dirty = false; }
        void MarkDirty() { m_dirty = true; }

        void SetUserData(void* data) { m_userData = data; }
        void* GetUserData() const { return m_userData; }

    protected:
        X3DNode* m_node = nullptr;
        std::string m_nodeName;
        std::string m_nodeTypeName;
        FieldCollection m_fields;
        bool m_dirty = false;
        void* m_userData = nullptr;

        virtual HRESULT SetupFields();
    };

    // --- X3DChildNodeImpl: Child node bridge ---
    class X3DChildNodeImpl : public X3DNodeImpl
    {
    public:
        X3DChildNodeImpl();
        virtual ~X3DChildNodeImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        X3DChildNode* GetChildNode() const { return m_childNode; }

        bool IsVisible() const { return m_visible; }
        void SetVisible(bool visible) { m_visible = visible; }

        X3DChildNodeImpl* GetParent() const { return m_parent; }
        void SetParent(X3DChildNodeImpl* parent) { m_parent = parent; }

        void AddChild(X3DChildNodeImpl* child);
        void RemoveChild(X3DChildNodeImpl* child);
        UINT GetNumChildren() const { return static_cast<UINT>(m_children.size()); }
        X3DChildNodeImpl* GetChild(UINT index) const;

        virtual void Traverse(std::function<void(X3DChildNodeImpl*, int)> visitor, int depth = 0);

    protected:
        X3DChildNode* m_childNode = nullptr;
        bool m_visible = true;
        X3DChildNodeImpl* m_parent = nullptr;
        std::vector<X3DChildNodeImpl*> m_children;

        HRESULT SetupFields() override;
    };

    // --- X3DChildObjectImpl: Child object bridge ---
    class X3DChildObjectImpl : public X3DNodeImpl
    {
    public:
        X3DChildObjectImpl();
        virtual ~X3DChildObjectImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

    protected:
        X3DChildNode* m_childObject = nullptr;
    };

    // --- X3DBoundedObjectImpl: Bounding volume bridge ---
    class X3DBoundedObjectImpl
    {
    public:
        X3DBoundedObjectImpl();
        virtual ~X3DBoundedObjectImpl();

        bool IsBounded() const { return m_bounded; }
        void SetBounded(bool bounded) { m_bounded = bounded; }

        void SetBoundingBox(const Vec3& center, const Vec3& extents);
        void GetBoundingBox(Vec3& center, Vec3& extents) const;

        bool IsCenterValid() const { return m_centerValid; }
        void InvalidateCenter() { m_centerValid = false; }

    protected:
        bool m_bounded = true;
        bool m_centerValid = false;
        Vec3 m_bboxCenter;
        Vec3 m_bboxExtents;
    };

} // namespace HMREngine
