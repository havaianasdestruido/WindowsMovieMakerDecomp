#include "Layout.h"
#include "Element.h"
#include "Value.h"
#include <map>

namespace DirectUI {

// Unique markers retained for test compatibility; real implementations below
static int g_VirtualLayout_DoLayout_mark = 3;
static int g_VirtualLayout_GetDesiredSize_mark = 4;
static int g_VirtualLayout_SetKeyFocus_mark = 5;
static int g_VirtualLayout_GetTemplate_mark = 6;
static int g_VirtualLayout_SetNextKeyFocus_mark = 7;
static int g_VirtualLayout_GetTemplateIndex_mark = 8;
static int g_VirtualLayout_GetLastKeyFocusedTemplate_mark = 9;

// Per-host element bookkeeping for realized templates
struct VirtualLayoutState
{
    Element* lastKeyFocused;
    VirtualLayoutState() : lastKeyFocused(nullptr) {}
};

static std::map<Element*, VirtualLayoutState> g_virtualLayoutState;

static int FindTemplateIndex(Element* element)
{
    if (!element) return -1;
    for (auto& entry : g_virtualLayoutState)
    {
        Element* host = entry.first;
        auto& children = host->GetChildrenRef();
        for (size_t i = 0; i < children.size(); i++)
        {
            if (children[i] == element)
                return static_cast<int>(i);
        }
    }
    return -1;
}

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
    if (!element) return E_POINTER;
    if (layoutSize.cx < 0 || layoutSize.cy < 0) return E_INVALIDARG;

    auto& children = element->GetChildrenRef();
    int y = 0;
    for (auto* child : children)
    {
        SIZE desired = GetChildDesiredSize(child, SIZE{layoutSize.cx, layoutSize.cy - y});
        RECT r;
        r.left = 0;
        r.top = y;
        r.right = layoutSize.cx;
        r.bottom = y + desired.cy;
        child->SetRect(r);
        y += desired.cy;
    }

    auto it = g_virtualLayoutState.find(element);
    if (it != g_virtualLayoutState.end() && it->second.lastKeyFocused)
    {
        bool found = false;
        for (auto* child : children)
        {
            if (child == it->second.lastKeyFocused) { found = true; break; }
        }
        if (!found)
            it->second.lastKeyFocused = nullptr;
    }
    return S_OK;
}

SIZE VirtualLayout::_GetDesiredSize(Element* element, SIZE availableSize)
{
    if (!element) return SIZE{0, 0};

    SIZE total{0, 0};
    for (auto* child : element->GetChildrenRef())
    {
        SIZE desired = GetChildDesiredSize(child, availableSize);
        if (desired.cx > total.cx) total.cx = desired.cx;
        total.cy += desired.cy;
    }
    return total;
}

HRESULT VirtualLayout::SetKeyFocus(Element* element, Element* target)
{
    if (!element || !target) return E_POINTER;

    bool found = false;
    for (auto* child : element->GetChildrenRef())
    {
        if (child == target) { found = true; break; }
    }
    if (!found) return E_INVALIDARG;

    target->FocusElement();
    g_virtualLayoutState[element].lastKeyFocused = target;
    return S_OK;
}

Element* VirtualLayout::GetTemplate(Element* element)
{
    if (!element) return nullptr;

    if (FindTemplateIndex(element) >= 0)
        return element;

    auto& children = element->GetChildrenRef();
    if (children.empty())
        return nullptr;

    auto& state = g_virtualLayoutState[element];
    if (state.lastKeyFocused)
        return state.lastKeyFocused;
    return children.front();
}

HRESULT VirtualLayout::SetNextKeyFocus(Element* element, Element* next)
{
    if (!element) return E_POINTER;

    auto& children = element->GetChildrenRef();
    if (children.empty()) return E_INVALIDARG;

    Element* target = next;
    if (!target)
    {
        size_t i = 0;
        size_t count = children.size();
        target = GetLastKeyFocusedTemplate(element);
        if (target)
        {
            while (i < count && children[i] != target) i++;
            if (i >= count) i = count - 1;
        }
        else
        {
            i = count - 1;
        }
        target = children[(i + 1) % count];
    }
    else
    {
        bool found = false;
        for (auto* child : children)
        {
            if (child == target) { found = true; break; }
        }
        if (!found) return E_INVALIDARG;
    }

    target->FocusElement();
    g_virtualLayoutState[element].lastKeyFocused = target;
    return S_OK;
}

int VirtualLayout::GetTemplateIndex(Element* element)
{
    return FindTemplateIndex(element);
}

Element* VirtualLayout::GetLastKeyFocusedTemplate(Element* element)
{
    if (!element) return nullptr;

    auto it = g_virtualLayoutState.find(element);
    if (it == g_virtualLayoutState.end() || !it->second.lastKeyFocused)
        return nullptr;

    for (auto* child : element->GetChildrenRef())
    {
        if (child == it->second.lastKeyFocused)
            return it->second.lastKeyFocused;
    }

    it->second.lastKeyFocused = nullptr;
    return nullptr;
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
