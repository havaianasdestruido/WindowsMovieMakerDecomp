#pragma once
#include <windows.h>
#include <ole2.h>
#include <map>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4251) // STL types need dll-interface

namespace DirectUI {

struct PropertyInfo {
    const wchar_t* name;
};

class Value;

class Layout;

class Element;

struct EventInfo {
    const wchar_t* name;
};

typedef HRESULT (CALLBACK *EventCallback)(Element* sender, Value** args, int numArgs);

class __declspec(dllexport) Element {
public:
    static Element* g_focusedElement;
    static void SetFocusElement(Element* el);

public:
    virtual ~Element();

    // Child management
    HRESULT Add(Element* child);
    HRESULT Insert(int index, Element* child);
    HRESULT Remove(Element* child);
    void DestroyAll();

    // Property system
    Value* GetValue(PropertyInfo const* prop);
    HRESULT SetValue(PropertyInfo const* prop, Value* value);

    // Events
    HRESULT FireEvent(int eventId, int numArgs, Value** args);

    // Lifecycle
    void StartDefer();
    HRESULT EndDefer();
    HRESULT EnableElement(bool enable);

    // Navigation
    Element* FindDescendent(int id);
    bool IsDescendent(Element* ancestor);
    HRESULT EnsureVisible();

    // Layout
    void _UpdateDesiredSize();
    void _UpdateLayoutSize();
    void _UpdateLayoutPosition();

    // Data context
    Value* GetDataContext();
    HRESULT SetDataContext(Value* context);

    // Layer
    int GetLayer();

    // Behaviors
    HRESULT QIBehaviors(REFIID riid, void** ppv);

    // Commands
    HRESULT ExecCmd(UINT cmdId, UINT cmdContext, Value* arg);

    // Coordinate mapping
    HRESULT MapElementPoint(Element* from, POINT* pt);

    // Device context
    HDC GetElementDC(HDC hdc, RECT* rect);
    HRESULT ReleaseElementDC(HDC hdc);

    // Focus
    HRESULT FocusElement();

    // Event listener system
    HRESULT AddEventListener(EventInfo* event, EventCallback callback);
    HRESULT RemoveEventListener(EventInfo* event, EventCallback callback);
    HRESULT FireEventInfo(EventInfo* event, int numArgs, Value** args);

    // ID helpers
    void SetID(int id);
    int GetID();

    // Input handling
    virtual HRESULT OnMouseMove(POINT pt, int mouseButton);
    virtual HRESULT OnMouseClick(POINT pt, int mouseButton);
    virtual HRESULT OnMouseDoubleClick(POINT pt, int mouseButton);
    virtual HRESULT OnKeyDown(UINT vk);
    virtual HRESULT OnKeyUp(UINT vk);
    virtual HRESULT OnChar(wchar_t ch);

    // Rendering / painting
    virtual HRESULT Paint(HDC hdc, RECT const* rcPaint);
    virtual HRESULT OnPaint(HDC hdc, RECT const* rcPaint);
    HRESULT Invalidate();
    HRESULT InvalidateRect(RECT const* rc);

    // Static property pointers
    static PropertyInfo* FontSizeProp;
    static PropertyInfo* LayoutPosProp;
    static PropertyInfo* IDProp;
    static PropertyInfo* DirectionProp;
    static PropertyInfo* DesiredSizeProp;
    static PropertyInfo* MinSizeProp;
    static PropertyInfo* ExtentProp;
    static PropertyInfo* ChildrenProp;
    static PropertyInfo* FontStyleProp;
    static PropertyInfo* FontWeightProp;
    static PropertyInfo* EnabledProp;
    static PropertyInfo* VisibleProp;
    static PropertyInfo* AccNameProp;
    static PropertyInfo* ContentProp;
    static PropertyInfo* KeyFocusedProp;
    static PropertyInfo* SizeInLayoutProp;
    static PropertyInfo* ParentProp;
    static PropertyInfo* ForegroundProp;
    static PropertyInfo* BackgroundProp;
    static PropertyInfo* ActiveProp;
    static PropertyInfo* MouseWithinProp;
    static PropertyInfo* TooltipProp;
    static PropertyInfo* WidthProp;
    static PropertyInfo* HeightProp;
    static PropertyInfo* XProp;
    static PropertyInfo* YProp;
    static PropertyInfo* KeyWithinProp;
    static PropertyInfo* BorderColorProp;
    static PropertyInfo* PaddingProp;
    static PropertyInfo* SelectedProp;
    static PropertyInfo* MouseFocusedProp;
    static PropertyInfo* ClassProp;
    static PropertyInfo* AccDescProp;
    static PropertyInfo* AccRoleProp;
    static PropertyInfo* AccessibleProp;
    static PropertyInfo* ContentAlignProp;
    static PropertyInfo* FontFaceProp;
    static PropertyInfo* LocationProp;
    static PropertyInfo* PosInLayoutProp;
    static PropertyInfo* TooltipString;
    static PropertyInfo* WrapKeyboardNavigateProp;
    static PropertyInfo* AnimationsEnabledProp;
    static PropertyInfo* TemplateSourceProp;
    static PropertyInfo* LayoutModeInterfaceProp;
    static PropertyInfo* LayoutProp;

    const RECT& GetRect() const { return m_rect; }
    void SetRect(const RECT& r) { m_rect = r; }
    Layout* GetLayout();
    std::vector<Element*>& GetChildrenRef() { return m_children; }

protected:
    std::vector<Element*> m_children;
    Element* m_parent = nullptr;
    std::map<PropertyInfo const*, Value*> m_properties;
    Value* m_dataContext = nullptr;
    int m_layer = 0;
    int m_deferCount = 0;
    bool m_deferPending = false;
    RECT m_rect = {};        // Bounding rectangle (position + size)
    bool m_visible = true;   // Visibility flag
    bool m_dirty = true;     // Needs repaint

    void _DestroyDC();
    HDC m_hdcCache = nullptr;
    HBITMAP m_hbmCache = nullptr;
    SIZE m_cacheSize = {};
    RECT m_dirtyRect = {};
    std::multimap<EventInfo*, EventCallback> m_eventListeners;
};

#pragma warning(pop)

} // namespace DirectUI
