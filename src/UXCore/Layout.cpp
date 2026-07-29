#include "Layout.h"
#include "Element.h"
#include "Value.h"

namespace DirectUI {

// Unique markers to prevent COMDAT folding of identical stub bodies
static int g_FillLayout_DoLayout_mark = 1;
static int g_FillLayout_GetDesiredSize_mark = 2;
static int g_VirtualLayout_DoLayout_mark = 3;
static int g_VirtualLayout_GetDesiredSize_mark = 4;
static int g_VirtualLayout_SetKeyFocus_mark = 5;
static int g_VirtualLayout_GetTemplate_mark = 6;
static int g_VirtualLayout_SetNextKeyFocus_mark = 7;
static int g_VirtualLayout_GetTemplateIndex_mark = 8;
static int g_VirtualLayout_GetLastKeyFocusedTemplate_mark = 9;
static int g_BorderLayout_DoLayout_mark = 10;
static int g_GridLayout_DoLayout_mark = 11;
static int g_RowLayout_DoLayout_mark = 12;
static int g_FlowLayout_DoLayout_mark = 13;
static int g_NineGridLayout_DoLayout_mark = 14;

// Layout
Layout::~Layout()
{
}

// FillLayout
FillLayout* FillLayout::Create()
{
    return new FillLayout();
}

HRESULT FillLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(layoutSize);
    return (g_FillLayout_DoLayout_mark > 0) ? S_OK : E_FAIL;
}

SIZE FillLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(availableSize);
    return (g_FillLayout_GetDesiredSize_mark > 0) ? SIZE{ 0, 0 } : SIZE{ -1, -1 };
}

// VirtualLayout property definitions
PropertyInfo g_TemplateSizeProp = { L"TemplateSize" };
PropertyInfo g_DataSourceProp = { L"DataSource" };
PropertyInfo g_AnimationsEnabledPropVL = { L"AnimationsEnabled" };
PropertyInfo g_LayoutModeInterfacePropVL = { L"LayoutModeInterface" };
PropertyInfo g_TemplateSourcePropVL = { L"TemplateSource" };

PropertyInfo* VirtualLayout::TemplateSizeProp = &g_TemplateSizeProp;
PropertyInfo* VirtualLayout::DataSourceProp = &g_DataSourceProp;
PropertyInfo* VirtualLayout::AnimationsEnabledProp = &g_AnimationsEnabledPropVL;
PropertyInfo* VirtualLayout::LayoutModeInterfaceProp = &g_LayoutModeInterfacePropVL;
PropertyInfo* VirtualLayout::TemplateSourceProp = &g_TemplateSourcePropVL;

HRESULT VirtualLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(layoutSize);
    return (g_VirtualLayout_DoLayout_mark > 0) ? S_OK : E_FAIL;
}

SIZE VirtualLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(availableSize);
    return (g_VirtualLayout_GetDesiredSize_mark > 0) ? SIZE{ 0, 0 } : SIZE{ -1, -1 };
}

HRESULT VirtualLayout::SetKeyFocus(Element* element, Element* target)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(target);
    return (g_VirtualLayout_SetKeyFocus_mark > 0) ? S_OK : E_FAIL;
}

Element* VirtualLayout::GetTemplate(Element* element)
{
    UNREFERENCED_PARAMETER(element);
    return (g_VirtualLayout_GetTemplate_mark > 0) ? nullptr : element;
}

HRESULT VirtualLayout::SetNextKeyFocus(Element* element, Element* next)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(next);
    return (g_VirtualLayout_SetNextKeyFocus_mark > 0) ? S_OK : E_FAIL;
}

int VirtualLayout::GetTemplateIndex(Element* element)
{
    UNREFERENCED_PARAMETER(element);
    return (g_VirtualLayout_GetTemplateIndex_mark > 0) ? -1 : 0;
}

Element* VirtualLayout::GetLastKeyFocusedTemplate(Element* element)
{
    UNREFERENCED_PARAMETER(element);
    return (g_VirtualLayout_GetLastKeyFocusedTemplate_mark > 0) ? nullptr : element;
}

// BorderLayout
HRESULT BorderLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(layoutSize);
    return (g_BorderLayout_DoLayout_mark > 0) ? S_OK : E_FAIL;
}

SIZE BorderLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(availableSize);
    return SIZE{ 0, 0 };
}

// GridLayout
HRESULT GridLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(layoutSize);
    return (g_GridLayout_DoLayout_mark > 0) ? S_OK : E_FAIL;
}

SIZE GridLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(availableSize);
    return SIZE{ 0, 0 };
}

// RowLayout
HRESULT RowLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(layoutSize);
    return (g_RowLayout_DoLayout_mark > 0) ? S_OK : E_FAIL;
}

SIZE RowLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(availableSize);
    return SIZE{ 0, 0 };
}

// FlowLayout
HRESULT FlowLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(layoutSize);
    return (g_FlowLayout_DoLayout_mark > 0) ? S_OK : E_FAIL;
}

SIZE FlowLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(availableSize);
    return SIZE{ 0, 0 };
}

// NineGridLayout
HRESULT NineGridLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(layoutSize);
    return (g_NineGridLayout_DoLayout_mark > 0) ? S_OK : E_FAIL;
}

SIZE NineGridLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    UNREFERENCED_PARAMETER(availableSize);
    return SIZE{ 0, 0 };
}

} // namespace DirectUI
