#include "Element.h"
#include "Value.h"

namespace DirectUI {

// Unique markers to prevent COMDAT folding of identical stub bodies
static int g_FireEvent_mark = 1;
static int g_ExecCmd_mark = 2;
static int g_EnsureVisible_mark = 3;
static int g_FocusElement_mark = 4;
static int g_MapElementPoint_mark = 5;
static int g_FindDescendent_mark = 6;
static int g_EnableElement_mark = 7;
static int g_ReleaseElementDC_mark = 8;
static int g_GetElementDC_mark = 9;
static int g_EndDefer_mark = 10;
static int g_UpdateDesiredSize_mark = 11;
static int g_UpdateLayoutSize_mark = 12;
static int g_UpdateLayoutPosition_mark = 13;

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

// Destructor
Element::~Element()
{
    DestroyAll();
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
    if (value)
        m_properties[prop] = value;
    else
        m_properties.erase(prop);
    return S_OK;
}

// Events
HRESULT Element::FireEvent(int eventId, int numArgs, Value** args)
{
    UNREFERENCED_PARAMETER(eventId);
    UNREFERENCED_PARAMETER(numArgs);
    UNREFERENCED_PARAMETER(args);
    return (g_FireEvent_mark > 0) ? E_NOTIMPL : S_OK;
}

// Lifecycle
void Element::StartDefer()
{
    m_deferCount++;
}

HRESULT Element::EndDefer()
{
    if (m_deferCount > 0)
        m_deferCount--;
    return S_OK;
}

HRESULT Element::EnableElement(bool enable)
{
    UNREFERENCED_PARAMETER(enable);
    return (g_EnableElement_mark > 0) ? S_OK : E_FAIL;
}

// Navigation
Element* Element::FindDescendent(int id)
{
    UNREFERENCED_PARAMETER(id);
    return (g_FindDescendent_mark > 0) ? nullptr : this;
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
    return (g_EnsureVisible_mark > 0) ? E_NOTIMPL : S_OK;
}

// Layout
void Element::_UpdateDesiredSize()
{
    if (g_UpdateDesiredSize_mark < 0) m_deferCount = 0;
}

void Element::_UpdateLayoutSize()
{
    if (g_UpdateLayoutSize_mark < 0) m_layer = 0;
}

void Element::_UpdateLayoutPosition()
{
    if (g_UpdateLayoutPosition_mark < 0) m_parent = nullptr;
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
    UNREFERENCED_PARAMETER(cmdId);
    UNREFERENCED_PARAMETER(cmdContext);
    UNREFERENCED_PARAMETER(arg);
    return (g_ExecCmd_mark > 0) ? E_NOTIMPL : S_OK;
}

// Coordinate mapping
HRESULT Element::MapElementPoint(Element* from, POINT* pt)
{
    UNREFERENCED_PARAMETER(from);
    UNREFERENCED_PARAMETER(pt);
    return (g_MapElementPoint_mark > 0) ? E_NOTIMPL : S_OK;
}

// Device context
HDC Element::GetElementDC(HDC hdc, RECT* rect)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(rect);
    return (g_GetElementDC_mark > 0) ? nullptr : NULL;
}

HRESULT Element::ReleaseElementDC(HDC hdc)
{
    UNREFERENCED_PARAMETER(hdc);
    return (g_ReleaseElementDC_mark > 0) ? S_OK : E_FAIL;
}

// Focus
HRESULT Element::FocusElement()
{
    return (g_FocusElement_mark > 0) ? E_NOTIMPL : S_OK;
}

} // namespace DirectUI
