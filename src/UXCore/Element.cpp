#include "Element.h"
#include "Value.h"
#include "Layout.h"

namespace DirectUI {

// Unique markers to prevent COMDAT folding of identical stub bodies
static int g_EnableElement_mark = 7;

static int g_EndDefer_mark = 10;

// Property info definitions
PropertyInfo g_FontSizeProp =        { L"FontSize" };
PropertyInfo g_LayoutPosProp =       { L"LayoutPos" };
PropertyInfo g_IDProp =              { L"ID" };
PropertyInfo g_DirectionProp =       { L"Direction" };
PropertyInfo g_DesiredSizeProp =     { L"DesiredSize" };
PropertyInfo g_MinSizeProp =         { L"MinSize" };
PropertyInfo g_ExtentProp =          { L"Extent" };
PropertyInfo g_ChildrenProp =        { L"Children" };
PropertyInfo g_FontStyleProp =       { L"FontStyle" };
PropertyInfo g_FontWeightProp =      { L"FontWeight" };
PropertyInfo g_EnabledProp =         { L"Enabled" };
PropertyInfo g_VisibleProp =         { L"Visible" };
PropertyInfo g_AccNameProp =         { L"AccName" };
PropertyInfo g_ContentProp =         { L"Content" };
PropertyInfo g_KeyFocusedProp =      { L"KeyFocused" };
PropertyInfo g_SizeInLayoutProp =    { L"SizeInLayout" };
PropertyInfo g_ParentProp =          { L"Parent" };
PropertyInfo g_ForegroundProp =      { L"Foreground" };
PropertyInfo g_BackgroundProp =      { L"Background" };
PropertyInfo g_ActiveProp =          { L"Active" };
PropertyInfo g_MouseWithinProp =     { L"MouseWithin" };
PropertyInfo g_TooltipProp =         { L"Tooltip" };
PropertyInfo g_WidthProp =           { L"Width" };
PropertyInfo g_HeightProp =          { L"Height" };
PropertyInfo g_XProp =               { L"X" };
PropertyInfo g_YProp =               { L"Y" };
PropertyInfo g_KeyWithinProp =       { L"KeyWithin" };
PropertyInfo g_BorderColorProp =     { L"BorderColor" };
PropertyInfo g_PaddingProp =         { L"Padding" };
PropertyInfo g_SelectedProp =        { L"Selected" };
PropertyInfo g_MouseFocusedProp =    { L"MouseFocused" };
PropertyInfo g_ClassProp =           { L"Class" };
PropertyInfo g_AccDescProp =         { L"AccDesc" };
PropertyInfo g_AccRoleProp =         { L"AccRole" };
PropertyInfo g_AccessibleProp =      { L"Accessible" };
PropertyInfo g_ContentAlignProp =    { L"ContentAlign" };
PropertyInfo g_FontFaceProp =        { L"FontFace" };
PropertyInfo g_LocationProp =        { L"Location" };
PropertyInfo g_PosInLayoutProp =     { L"PosInLayout" };
PropertyInfo g_TooltipString =       { L"TooltipString" };
PropertyInfo g_WrapKeyboardNavigateProp = { L"WrapKeyboardNavigate" };
PropertyInfo g_AnimationsEnabledProp = { L"AnimationsEnabled" };
PropertyInfo g_TemplateSourceProp =  { L"TemplateSource" };
PropertyInfo g_LayoutModeInterfaceProp = { L"LayoutModeInterface" };
PropertyInfo g_LayoutProp =          { L"Layout" };

PropertyInfo* Element::FontSizeProp = &g_FontSizeProp;
PropertyInfo* Element::LayoutPosProp = &g_LayoutPosProp;
PropertyInfo* Element::IDProp = &g_IDProp;
PropertyInfo* Element::DirectionProp = &g_DirectionProp;
PropertyInfo* Element::DesiredSizeProp = &g_DesiredSizeProp;
PropertyInfo* Element::MinSizeProp = &g_MinSizeProp;
PropertyInfo* Element::ExtentProp = &g_ExtentProp;
PropertyInfo* Element::ChildrenProp = &g_ChildrenProp;
PropertyInfo* Element::FontStyleProp = &g_FontStyleProp;
PropertyInfo* Element::FontWeightProp = &g_FontWeightProp;
PropertyInfo* Element::EnabledProp = &g_EnabledProp;
PropertyInfo* Element::VisibleProp = &g_VisibleProp;
PropertyInfo* Element::AccNameProp = &g_AccNameProp;
PropertyInfo* Element::ContentProp = &g_ContentProp;
PropertyInfo* Element::KeyFocusedProp = &g_KeyFocusedProp;
PropertyInfo* Element::SizeInLayoutProp = &g_SizeInLayoutProp;
PropertyInfo* Element::ParentProp = &g_ParentProp;
PropertyInfo* Element::ForegroundProp = &g_ForegroundProp;
PropertyInfo* Element::BackgroundProp = &g_BackgroundProp;
PropertyInfo* Element::ActiveProp = &g_ActiveProp;
PropertyInfo* Element::MouseWithinProp = &g_MouseWithinProp;
PropertyInfo* Element::TooltipProp = &g_TooltipProp;
PropertyInfo* Element::WidthProp = &g_WidthProp;
PropertyInfo* Element::HeightProp = &g_HeightProp;
PropertyInfo* Element::XProp = &g_XProp;
PropertyInfo* Element::YProp = &g_YProp;
PropertyInfo* Element::KeyWithinProp = &g_KeyWithinProp;
PropertyInfo* Element::BorderColorProp = &g_BorderColorProp;
PropertyInfo* Element::PaddingProp = &g_PaddingProp;
PropertyInfo* Element::SelectedProp = &g_SelectedProp;
PropertyInfo* Element::MouseFocusedProp = &g_MouseFocusedProp;
PropertyInfo* Element::ClassProp = &g_ClassProp;
PropertyInfo* Element::AccDescProp = &g_AccDescProp;
PropertyInfo* Element::AccRoleProp = &g_AccRoleProp;
PropertyInfo* Element::AccessibleProp = &g_AccessibleProp;
PropertyInfo* Element::ContentAlignProp = &g_ContentAlignProp;
PropertyInfo* Element::FontFaceProp = &g_FontFaceProp;
PropertyInfo* Element::LocationProp = &g_LocationProp;
PropertyInfo* Element::PosInLayoutProp = &g_PosInLayoutProp;
PropertyInfo* Element::TooltipString = &g_TooltipString;
PropertyInfo* Element::WrapKeyboardNavigateProp = &g_WrapKeyboardNavigateProp;
PropertyInfo* Element::AnimationsEnabledProp = &g_AnimationsEnabledProp;
PropertyInfo* Element::TemplateSourceProp = &g_TemplateSourceProp;
PropertyInfo* Element::LayoutModeInterfaceProp = &g_LayoutModeInterfaceProp;
PropertyInfo* Element::LayoutProp = &g_LayoutProp;

static EventInfo g_FocusGainedEvent = { L"FocusGained" };
static EventInfo g_FocusLostEvent = { L"FocusLost" };
static EventInfo g_CommandEvent = { L"Command" };

// Focus tracking
Element* Element::g_focusedElement = nullptr;

void Element::SetFocusElement(Element* el)
{
    if (g_focusedElement == el)
        return;

    if (g_focusedElement)
    {
        g_focusedElement->SetValue(KeyFocusedProp, Value::CreateBool(false));
        g_focusedElement->FireEventInfo(&g_FocusLostEvent, 0, nullptr);
    }

    g_focusedElement = el;
    if (g_focusedElement)
    {
        g_focusedElement->SetValue(KeyFocusedProp, Value::CreateBool(true));
        g_focusedElement->FireEventInfo(&g_FocusGainedEvent, 0, nullptr);
    }
}

// Destructor
Element::~Element()
{
    _DestroyDC();
    DestroyAll();
}

void Element::_DestroyDC()
{
    if (m_hbmCache)
    {
        DeleteObject(m_hbmCache);
        m_hbmCache = nullptr;
    }
    if (m_hdcCache)
    {
        DeleteDC(m_hdcCache);
        m_hdcCache = nullptr;
    }
    m_cacheSize = {};
}

// Child management
HRESULT Element::Add(Element* child)
{
    if (!child) return E_POINTER;
    m_children.push_back(child);
    child->m_parent = this;
    return S_OK;
}

HRESULT Element::Insert(int index, Element* child)
{
    if (!child) return E_POINTER;
    if (index < 0 || index > static_cast<int>(m_children.size()))
        return E_INVALIDARG;
    m_children.insert(m_children.begin() + index, child);
    child->m_parent = this;
    return S_OK;
}

HRESULT Element::Remove(Element* child)
{
    if (!child) return E_POINTER;
    for (auto it = m_children.begin(); it != m_children.end(); ++it)
    {
        if (*it == child)
        {
            m_children.erase(it);
            child->m_parent = nullptr;
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

void Element::DestroyAll()
{
    for (auto* child : m_children)
    {
        child->DestroyAll();
    }
    m_children.clear();
}

// Property system
Value* Element::GetValue(PropertyInfo const* prop)
{
    if (!prop) return Value::pvNull;
    auto it = m_properties.find(prop);
    if (it != m_properties.end())
        return it->second;
    return Value::pvUnset;
}

HRESULT Element::SetValue(PropertyInfo const* prop, Value* value)
{
    if (!prop) return E_POINTER;
    // Release existing value to avoid leak
    auto it = m_properties.find(prop);
    if (it != m_properties.end()) {
        if (it->second) it->second->Release();
    }
    if (value) {
        value->AddRef();
        m_properties[prop] = value;
    } else {
        m_properties.erase(prop);
    }
    return S_OK;
}

// Events
HRESULT Element::FireEvent(int eventId, int numArgs, Value** args)
{
    for (auto* child : m_children)
        child->FireEvent(eventId, numArgs, args);
    UNREFERENCED_PARAMETER(eventId);
    return S_OK;
}

HRESULT Element::AddEventListener(EventInfo* event, EventCallback callback)
{
    if (!event || !callback) return E_POINTER;
    m_eventListeners.insert({event, callback});
    return S_OK;
}

HRESULT Element::RemoveEventListener(EventInfo* event, EventCallback callback)
{
    if (!event || !callback) return E_POINTER;
    auto range = m_eventListeners.equal_range(event);
    for (auto it = range.first; it != range.second; )
    {
        if (it->second == callback)
            it = m_eventListeners.erase(it);
        else
            ++it;
    }
    return S_OK;
}

HRESULT Element::FireEventInfo(EventInfo* event, int numArgs, Value** args)
{
    if (!event) return E_POINTER;
    HRESULT hr = S_OK;
    auto range = m_eventListeners.equal_range(event);
    for (auto it = range.first; it != range.second; ++it)
    {
        HRESULT hrEntry = it->second(this, args, numArgs);
        if (FAILED(hrEntry))
            hr = hrEntry;
    }
    return hr;
}

void Element::SetID(int id)
{
    SetValue(IDProp, Value::CreateInt(id));
}

int Element::GetID()
{
    Value* v = GetValue(IDProp);
    return (v && v->GetType() == Value::Int) ? v->GetInt() : 0;
}

// Lifecycle
void Element::StartDefer()
{
    if (m_deferCount++ == 0)
        m_deferPending = true;
}

HRESULT Element::EndDefer()
{
    if (m_deferCount == 0)
        return E_UNEXPECTED;
    if (--m_deferCount == 0 && m_deferPending)
    {
        m_deferPending = false;
        _UpdateDesiredSize();
        _UpdateLayoutSize();
        _UpdateLayoutPosition();
        Invalidate();
    }
    return S_OK;
}

HRESULT Element::EnableElement(bool enable)
{
    Value* cur = GetValue(EnabledProp);
    bool enabled = (!cur || cur->GetType() != Value::Bool) ? true : cur->GetBool();
    if (enabled == enable)
        return S_OK;

    HRESULT hr = SetValue(EnabledProp, enable ? Value::pvBoolTrue : Value::pvBoolFalse);
    if (FAILED(hr))
        return hr;

    for (auto* child : m_children)
    {
        HRESULT hrChild = child->EnableElement(enable);
        if (FAILED(hrChild))
            hr = hrChild;
    }

    if (m_deferCount > 0)
    {
        m_deferPending = true;
        return hr;
    }

    _UpdateDesiredSize();
    _UpdateLayoutSize();
    _UpdateLayoutPosition();
    Invalidate();
    return hr;
}

// Navigation
Element* Element::FindDescendent(int id)
{
    if (GetID() == id)
        return this;
    for (auto* child : m_children)
    {
        Element* found = child->FindDescendent(id);
        if (found)
            return found;
    }
    return nullptr;
}

bool Element::IsDescendent(Element* ancestor)
{
    Element* current = m_parent;
    while (current)
    {
        if (current == ancestor)
            return true;
        current = current->m_parent;
    }
    return false;
}

HRESULT Element::EnsureVisible()
{
    if (!m_parent) return S_OK;
    RECT parentRect = m_parent->m_rect;
    if (m_rect.left < parentRect.left || m_rect.top < parentRect.top ||
        m_rect.right > parentRect.right || m_rect.bottom > parentRect.bottom)
    {
        m_parent->InvalidateRect(&m_rect);
    }
    return m_parent->EnsureVisible();
}

// Layout
void Element::_UpdateDesiredSize()
{
    Layout* layout = GetLayout();
    if (layout)
    {
        SIZE available;
        available.cx = m_rect.right - m_rect.left;
        available.cy = m_rect.bottom - m_rect.top;
        SIZE desired = layout->_GetDesiredSize(this, available);
        SetValue(DesiredSizeProp, Value::CreateSize(desired));
        return;
    }
    SIZE total = {0, 0};
    for (auto* child : m_children)
    {
        child->_UpdateDesiredSize();
        Value* dv = child->GetValue(DesiredSizeProp);
        if (dv && dv->GetType() == Value::Size)
        {
            SIZE cs = dv->GetSize();
            if (cs.cx > total.cx) total.cx = cs.cx;
            total.cy += cs.cy;
        }
    }
    if (total.cx == 0 && total.cy == 0)
    {
        total.cx = m_rect.right - m_rect.left;
        total.cy = m_rect.bottom - m_rect.top;
    }
    SetValue(DesiredSizeProp, Value::CreateSize(total));
}

void Element::_UpdateLayoutSize()
{
    Layout* layout = GetLayout();
    if (layout)
    {
        SIZE size;
        size.cx = m_rect.right - m_rect.left;
        size.cy = m_rect.bottom - m_rect.top;
        layout->_DoLayout(this, size);
    }
}

void Element::_UpdateLayoutPosition()
{
    Invalidate();
}

Layout* Element::GetLayout()
{
    Value* val = GetValue(LayoutProp);
    if (val && val->GetType() == Value::LayoutPtr)
        return val->GetLayout();
    return nullptr;
}

// Data context
Value* Element::GetDataContext()
{
    return m_dataContext ? m_dataContext : Value::pvNull;
}

HRESULT Element::SetDataContext(Value* context)
{
    m_dataContext = context;
    return S_OK;
}

// Layer
int Element::GetLayer()
{
    return m_layer;
}

// Behaviors
HRESULT Element::QIBehaviors(REFIID riid, void** ppv)
{
    UNREFERENCED_PARAMETER(riid);
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    return E_NOINTERFACE;
}

// Commands
HRESULT Element::ExecCmd(UINT cmdId, UINT cmdContext, Value* arg)
{
    Element* current = this;
    while (current)
    {
        Value* cmdArgs[2] = {
            Value::CreateInt(cmdId),
            arg ? arg : Value::pvNull
        };
        HRESULT hr = current->FireEventInfo(&g_CommandEvent, 2, cmdArgs);
        if (hr == S_OK)
            return S_OK;
        current = current->m_parent;
    }
    UNREFERENCED_PARAMETER(cmdContext);
    return S_FALSE;
}

// Coordinate mapping
HRESULT Element::MapElementPoint(Element* from, POINT* pt)
{
    if (!from || !pt) return E_POINTER;
    if (from == this) return S_OK;

    std::vector<Element*> fromPath;
    Element* cur = from;
    while (cur) {
        fromPath.push_back(cur);
        cur = cur->m_parent;
    }

    std::vector<Element*> thisPath;
    cur = this;
    while (cur) {
        thisPath.push_back(cur);
        cur = cur->m_parent;
    }

    int commonIdx = -1;
    int fromSize = (int)fromPath.size();
    int thisSize = (int)thisPath.size();
    for (int i = 0; i < fromSize && i < thisSize; i++) {
        if (fromPath[fromSize - 1 - i] == thisPath[thisSize - 1 - i])
            commonIdx = i;
        else
            break;
    }
    if (commonIdx == -1)
        return E_FAIL;

    int fromCommonIdx = fromSize - 1 - commonIdx;
    for (int i = 0; i < fromCommonIdx; i++) {
        pt->x += fromPath[i]->m_rect.left;
        pt->y += fromPath[i]->m_rect.top;
    }

    int thisCommonIdx = thisSize - 1 - commonIdx;
    for (int i = thisCommonIdx - 1; i >= 0; i--) {
        pt->x -= thisPath[i]->m_rect.left;
        pt->y -= thisPath[i]->m_rect.top;
    }

    return S_OK;
}

// Device context
HDC Element::GetElementDC(HDC hdc, RECT* rect)
{
    if (!rect)
        return nullptr;

    int width = rect->right - rect->left;
    int height = rect->bottom - rect->top;

    if (width <= 0 || height <= 0)
        return nullptr;

    if (m_hdcCache && m_cacheSize.cx == width && m_cacheSize.cy == height)
        return m_hdcCache;

    _DestroyDC();

    HDC refDC = hdc ? hdc : GetDC(nullptr);
    m_hdcCache = CreateCompatibleDC(refDC);
    if (m_hdcCache)
    {
        m_hbmCache = CreateCompatibleBitmap(refDC, width, height);
        if (m_hbmCache)
        {
            SelectObject(m_hdcCache, m_hbmCache);
            m_cacheSize.cx = width;
            m_cacheSize.cy = height;
        }
        else
        {
            DeleteDC(m_hdcCache);
            m_hdcCache = nullptr;
        }
    }

    if (!hdc)
        ReleaseDC(nullptr, refDC);

    return m_hdcCache;
}

HRESULT Element::ReleaseElementDC(HDC hdc)
{
    if (hdc == m_hdcCache)
        return S_OK;
    UNREFERENCED_PARAMETER(hdc);
    return S_OK;
}

// Focus
HRESULT Element::FocusElement()
{
    SetFocusElement(this);
    return S_OK;
}

HRESULT Element::OnMouseMove(POINT pt, int mouseButton)
{
    UNREFERENCED_PARAMETER(pt);
    UNREFERENCED_PARAMETER(mouseButton);
    return S_FALSE;
}

HRESULT Element::OnMouseClick(POINT pt, int mouseButton)
{
    UNREFERENCED_PARAMETER(pt);
    UNREFERENCED_PARAMETER(mouseButton);
    return S_FALSE;
}

HRESULT Element::OnMouseDoubleClick(POINT pt, int mouseButton)
{
    UNREFERENCED_PARAMETER(pt);
    UNREFERENCED_PARAMETER(mouseButton);
    return S_FALSE;
}

HRESULT Element::OnKeyDown(UINT vk)
{
    UNREFERENCED_PARAMETER(vk);
    return S_FALSE;
}

HRESULT Element::OnKeyUp(UINT vk)
{
    UNREFERENCED_PARAMETER(vk);
    return S_FALSE;
}

HRESULT Element::OnChar(wchar_t ch)
{
    UNREFERENCED_PARAMETER(ch);
    return S_FALSE;
}

// Rendering / painting
HRESULT Element::Paint(HDC hdc, RECT const* rcPaint)
{
    if (!m_visible)
        return S_OK;

    if (IsRectEmpty(&m_rect))
        return S_OK;

    RECT intersectRect;
    if (!IntersectRect(&intersectRect, rcPaint, &m_rect))
        return S_OK;

    HDC bufferDC = GetElementDC(hdc, const_cast<RECT*>(&m_rect));
    if (!bufferDC)
        return E_FAIL;

    OnPaint(bufferDC, &m_rect);

    BitBlt(hdc, m_rect.left, m_rect.top,
           m_rect.right - m_rect.left, m_rect.bottom - m_rect.top,
           bufferDC, 0, 0, SRCCOPY);

    for (auto* child : m_children)
    {
        child->Paint(hdc, rcPaint);
    }

    return S_OK;
}

HRESULT Element::OnPaint(HDC hdc, RECT const* rcPaint)
{
    Value* bg = GetValue(BackgroundProp);
    if (bg && bg->GetType() == Value::Color)
    {
        HBRUSH brush = CreateSolidBrush(bg->GetColor());
        if (brush)
        {
            FillRect(hdc, rcPaint, brush);
            DeleteObject(brush);
        }
    }

    Value* border = GetValue(BorderColorProp);
    if (border && border->GetType() == Value::Color)
    {
        HPEN pen = CreatePen(PS_SOLID, 1, border->GetColor());
        if (pen)
        {
            HGDIOBJ oldPen = SelectObject(hdc, pen);
            HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rcPaint->left, rcPaint->top,
                      rcPaint->right, rcPaint->bottom);
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(pen);
        }
    }

    return S_OK;
}

HRESULT Element::Invalidate()
{
    m_dirty = true;
    if (m_parent)
        m_parent->InvalidateRect(&m_rect);
    return S_OK;
}

HRESULT Element::InvalidateRect(RECT const* rc)
{
    UnionRect(&m_dirtyRect, &m_dirtyRect, rc);
    if (m_parent)
        m_parent->InvalidateRect(rc);
    return S_OK;
}

} // namespace DirectUI
