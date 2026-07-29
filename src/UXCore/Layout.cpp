#include "Layout.h"
#include "Element.h"
#include "Value.h"

namespace DirectUI {

// Unique markers to prevent COMDAT folding of identical stub bodies
static int g_VirtualLayout_DoLayout_mark = 3;
static int g_VirtualLayout_GetDesiredSize_mark = 4;
static int g_VirtualLayout_SetKeyFocus_mark = 5;
static int g_VirtualLayout_GetTemplate_mark = 6;
static int g_VirtualLayout_SetNextKeyFocus_mark = 7;
static int g_VirtualLayout_GetTemplateIndex_mark = 8;
static int g_VirtualLayout_GetLastKeyFocusedTemplate_mark = 9;

// Layout
Layout::~Layout()
{
}

// Helper: get a child's desired size
static SIZE GetChildDesiredSize(Element* child, SIZE available)
{
    Value* val = child->GetValue(Element::LayoutProp);
    if (val && val->GetType() == Value::LayoutPtr)
    {
        Layout* layout = val->GetLayout();
        if (layout)
            return layout->_GetDesiredSize(child, available);
    }
    SIZE s;
    RECT r = child->GetRect();
    int w = r.right - r.left;
    int h = r.bottom - r.top;
    s.cx = w > 0 ? w : 80;
    s.cy = h > 0 ? h : available.cy > 0 ? available.cy : 30;
    return s;
}

// FillLayout
FillLayout* FillLayout::Create()
{
    return new FillLayout();
}

HRESULT FillLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    auto& children = element->GetChildrenRef();
    RECT rc = {0, 0, layoutSize.cx, layoutSize.cy};
    for (auto* child : children)
    {
        child->SetRect(rc);
    }
    return S_OK;
}

SIZE FillLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    return availableSize;
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
    auto& children = element->GetChildrenRef();
    int left = 0, top = 0, right = layoutSize.cx, bottom = layoutSize.cy;

    for (auto* child : children)
    {
        Value* val = child->GetValue(Element::LayoutPosProp);
        int pos = (val && val->GetType() == Value::Int) ? val->GetInt() : 5;

        if (pos == 1)
        {
            SIZE desired = GetChildDesiredSize(child, SIZE{right - left, bottom - top});
            RECT r;
            r.left = left;
            r.top = top;
            r.right = left + desired.cx;
            r.bottom = bottom;
            child->SetRect(r);
            left += desired.cx;
        }
        else if (pos == 2)
        {
            SIZE desired = GetChildDesiredSize(child, SIZE{right - left, bottom - top});
            RECT r;
            r.left = left;
            r.top = top;
            r.right = right;
            r.bottom = top + desired.cy;
            child->SetRect(r);
            top += desired.cy;
        }
    }

    for (auto* child : children)
    {
        Value* val = child->GetValue(Element::LayoutPosProp);
        int pos = (val && val->GetType() == Value::Int) ? val->GetInt() : 5;

        if (pos == 3)
        {
            SIZE desired = GetChildDesiredSize(child, SIZE{right - left, bottom - top});
            RECT r;
            r.left = right - desired.cx;
            r.top = top;
            r.right = right;
            r.bottom = bottom;
            child->SetRect(r);
            right -= desired.cx;
        }
        else if (pos == 4)
        {
            SIZE desired = GetChildDesiredSize(child, SIZE{right - left, bottom - top});
            RECT r;
            r.left = left;
            r.top = bottom - desired.cy;
            r.right = right;
            r.bottom = bottom;
            child->SetRect(r);
            bottom -= desired.cy;
        }
    }

    for (auto* child : children)
    {
        Value* val = child->GetValue(Element::LayoutPosProp);
        int pos = (val && val->GetType() == Value::Int) ? val->GetInt() : 5;

        if (pos == 5 || pos == 6)
        {
            RECT r;
            r.left = left;
            r.top = top;
            r.right = right;
            r.bottom = bottom;
            child->SetRect(r);
        }
    }

    return S_OK;
}

SIZE BorderLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    auto& children = element->GetChildrenRef();
    int leftW = 0, rightW = 0, topH = 0, bottomH = 0;
    int centerW = 0, centerH = 0;

    for (auto* child : children)
    {
        Value* val = child->GetValue(Element::LayoutPosProp);
        int pos = (val && val->GetType() == Value::Int) ? val->GetInt() : 5;
        SIZE desired = GetChildDesiredSize(child, availableSize);

        switch (pos)
        {
        case 1: if (desired.cx > leftW) leftW = desired.cx; break;
        case 2: if (desired.cy > topH) topH = desired.cy; break;
        case 3: if (desired.cx > rightW) rightW = desired.cx; break;
        case 4: if (desired.cy > bottomH) bottomH = desired.cy; break;
        case 5:
        case 6:
        default:
            if (desired.cx > centerW) centerW = desired.cx;
            if (desired.cy > centerH) centerH = desired.cy;
            break;
        }
    }

    SIZE s;
    s.cx = leftW + rightW + centerW;
    s.cy = topH + bottomH + centerH;
    return s;
}

// GridLayout property definitions
PropertyInfo g_GridColumnsProp = { L"GridColumns" };
PropertyInfo* GridLayout::GridColumnsProp = &g_GridColumnsProp;

HRESULT GridLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    Value* val = element->GetValue(GridColumnsProp);
    int numCols = 1;
    if (val && val->GetType() == Value::Int)
        numCols = val->GetInt();
    if (numCols < 1) numCols = 1;

    auto& children = element->GetChildrenRef();
    int count = static_cast<int>(children.size());
    int numRows = (count + numCols - 1) / numCols;
    if (numRows < 1) numRows = 1;

    int cellW = layoutSize.cx / numCols;
    int cellH = layoutSize.cy / numRows;

    for (int i = 0; i < count; i++)
    {
        int row = i / numCols;
        int col = i % numCols;
        RECT r;
        r.left = col * cellW;
        r.top = row * cellH;
        r.right = (col + 1) * cellW;
        r.bottom = (row + 1) * cellH;
        children[i]->SetRect(r);
    }

    return S_OK;
}

SIZE GridLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    return availableSize;
}

// RowLayout property definitions
static PropertyInfo g_SpacingProp = { L"Spacing" };

HRESULT RowLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    Value* spacingVal = element->GetValue(&g_SpacingProp);
    int spacing = (spacingVal && spacingVal->GetType() == Value::Int) ? spacingVal->GetInt() : 0;

    auto& children = element->GetChildrenRef();
    int x = 0;
    for (auto* child : children)
    {
        SIZE desired = GetChildDesiredSize(child, SIZE{layoutSize.cx - x, layoutSize.cy});
        int w = desired.cx;
        if (x + w > layoutSize.cx && x > 0)
        {
            w = layoutSize.cx - x;
            if (w < 0) w = 0;
        }
        RECT r;
        r.left = x;
        r.top = 0;
        r.right = x + w;
        r.bottom = layoutSize.cy;
        child->SetRect(r);
        x += w + spacing;
    }

    return S_OK;
}

SIZE RowLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    Value* spacingVal = element->GetValue(&g_SpacingProp);
    int spacing = (spacingVal && spacingVal->GetType() == Value::Int) ? spacingVal->GetInt() : 0;

    auto& children = element->GetChildrenRef();
    int totalW = 0;
    int maxH = 0;
    int count = 0;
    for (auto* child : children)
    {
        SIZE desired = GetChildDesiredSize(child, availableSize);
        totalW += desired.cx;
        if (desired.cy > maxH) maxH = desired.cy;
        count++;
    }
    if (count > 1)
        totalW += spacing * (count - 1);

    SIZE s;
    s.cx = totalW;
    s.cy = maxH;
    return s;
}

// FlowLayout
HRESULT FlowLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    Value* spacingVal = element->GetValue(&g_SpacingProp);
    int spacing = (spacingVal && spacingVal->GetType() == Value::Int) ? spacingVal->GetInt() : 0;

    auto& children = element->GetChildrenRef();
    int x = 0, y = 0;
    int rowH = 0;

    for (auto* child : children)
    {
        SIZE desired = GetChildDesiredSize(child, SIZE{layoutSize.cx - x, layoutSize.cy - y});
        int w = desired.cx;
        int h = desired.cy;

        if (x + w > layoutSize.cx && x > 0)
        {
            y += rowH + spacing;
            x = 0;
            rowH = 0;
        }

        RECT r;
        r.left = x;
        r.top = y;
        r.right = x + w;
        r.bottom = y + h;
        child->SetRect(r);

        x += w + spacing;
        if (h > rowH) rowH = h;
    }

    return S_OK;
}

SIZE FlowLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(element);
    return availableSize;
}

// NineGridLayout property definitions
static PropertyInfo g_LeftMarginProp = { L"LeftMargin" };
static PropertyInfo g_TopMarginProp = { L"TopMargin" };
static PropertyInfo g_RightMarginProp = { L"RightMargin" };
static PropertyInfo g_BottomMarginProp = { L"BottomMargin" };

static int GetMargin(Element* element, PropertyInfo* prop, int def)
{
    Value* v = element->GetValue(prop);
    return (v && v->GetType() == Value::Int) ? v->GetInt() : def;
}

HRESULT NineGridLayout::_DoLayout(Element* element, SIZE layoutSize)
{
    int lm = GetMargin(element, &g_LeftMarginProp, 0);
    int tm = GetMargin(element, &g_TopMarginProp, 0);
    int rm = GetMargin(element, &g_RightMarginProp, 0);
    int bm = GetMargin(element, &g_BottomMarginProp, 0);

    int w = layoutSize.cx;
    int h = layoutSize.cy;

    int lr = lm;
    if (lr > w) lr = w;
    int rr = w - rm;
    if (rr < lr) rr = lr;
    int tb = tm;
    if (tb > h) tb = h;
    int bt = h - bm;
    if (bt < tb) bt = tb;

    RECT regions[9] = {
        {0, 0, lr, tb},
        {lr, 0, rr, tb},
        {rr, 0, w, tb},
        {0, tb, lr, bt},
        {lr, tb, rr, bt},
        {rr, tb, w, bt},
        {0, bt, lr, h},
        {lr, bt, rr, h},
        {rr, bt, w, h}
    };

    auto& children = element->GetChildrenRef();
    for (auto* child : children)
    {
        Value* val = child->GetValue(Element::LayoutPosProp);
        int pos = (val && val->GetType() == Value::Int) ? val->GetInt() : 5;
        if (pos < 1) pos = 1;
        if (pos > 9) pos = 9;
        child->SetRect(regions[pos - 1]);
    }

    return S_OK;
}

SIZE NineGridLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    UNREFERENCED_PARAMETER(availableSize);

    int lm = GetMargin(element, &g_LeftMarginProp, 0);
    int tm = GetMargin(element, &g_TopMarginProp, 0);
    int rm = GetMargin(element, &g_RightMarginProp, 0);
    int bm = GetMargin(element, &g_BottomMarginProp, 0);

    SIZE s;
    s.cx = lm + rm;
    s.cy = tm + bm;
    return s;
}

} // namespace DirectUI
