#include "Controls.h"
#include "Value.h"

namespace DirectUI {

// ─── ControlBase ────────────────────────────────────────────────────

ControlBase::~ControlBase() = default;

HRESULT ControlBase::AddHandler(EventInfo* event, EventCallback callback)
{
    if (!event || !callback) return E_POINTER;
    HandlerEntry entry;
    entry.event = event;
    entry.callback = callback;
    m_handlers.push_back(entry);
    return S_OK;
}

HRESULT ControlBase::RemoveHandler(EventInfo* event, EventCallback callback)
{
    if (!event || !callback) return E_POINTER;
    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it)
    {
        if (it->event == event && it->callback == callback)
        {
            m_handlers.erase(it);
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT ControlBase::FireEvent(EventInfo* event, int numArgs, Value** args)
{
    if (!event) return E_POINTER;
    HRESULT hr = S_OK;
    for (auto& entry : m_handlers)
    {
        if (entry.event == event)
        {
            HRESULT hrEntry = entry.callback(this, args, numArgs);
            if (FAILED(hrEntry))
                hr = hrEntry;
        }
    }
    return hr;
}

// ─── IClassInfo instances ───────────────────────────────────────────

static IClassInfo g_ButtonClass;
static IClassInfo g_ButtonTextClass;
static IClassInfo g_LabelClass;
static IClassInfo g_EditClass;
static IClassInfo g_CheckboxClass;
static IClassInfo g_ScrollBarClass;
static IClassInfo g_SliderClass;
static IClassInfo g_ComboboxClass;
static IClassInfo g_ThumbClass;
static IClassInfo g_HyperlinkClass;
static IClassInfo g_WLEditTClass;
static IClassInfo g_SelectorClass;

// ─── EventInfo instances ────────────────────────────────────────────

static EventInfo g_ClickEvent             = { L"Click" };
static EventInfo g_LostFocusEvent         = { L"LostFocus" };
static EventInfo g_ChangeEvent            = { L"Change" };
static EventInfo g_SelectionChangeEvent   = { L"SelectionChange" };
static EventInfo g_NavigateEvent          = { L"Navigate" };

// ─── Button ─────────────────────────────────────────────────────────

Button::~Button() = default;
IClassInfo* Button::Class = &g_ButtonClass;
PropertyInfo* Button::PressedProp = nullptr;
PropertyInfo* Button::FontSizeProp = nullptr;
EventInfo* Button::Click = &g_ClickEvent;

HRESULT Button::OnPaint(HDC hdc, RECT const* rcPaint)
{
    HBRUSH bgBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hdc, rcPaint, bgBrush);
    DeleteObject(bgBrush);

    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(128, 128, 128));
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, rcPaint->left, rcPaint->top, rcPaint->right, rcPaint->bottom);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(borderPen);

    Value* content = GetValue(ContentProp);
    if (content && content->GetType() == Value::String)
    {
        SetBkMode(hdc, TRANSPARENT);

        Value* pressed = GetValue(PressedProp);
        int offsetX = 0;
        int offsetY = 0;
        if (pressed && pressed->GetType() == Value::Bool && pressed->GetBool())
        {
            offsetX = 1;
            offsetY = 1;
        }

        RECT textRect = *rcPaint;
        InflateRect(&textRect, -4, -2);
        OffsetRect(&textRect, offsetX, offsetY);

        HFONT hFont = nullptr;
        HGDIOBJ oldFont = nullptr;

        Value* fontFace = GetValue(FontFaceProp);
        if (fontFace && fontFace->GetType() == Value::String)
        {
            hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                               fontFace->GetString());
        }
        else
        {
            hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        }
        oldFont = SelectObject(hdc, hFont);

        DrawText(hdc, content->GetString(), -1, &textRect,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, oldFont);
        if (fontFace && fontFace->GetType() == Value::String)
            DeleteObject(hFont);
    }

    return S_OK;
}

// ─── ButtonText ─────────────────────────────────────────────────────

ButtonText::~ButtonText() = default;
IClassInfo* ButtonText::Class = &g_ButtonTextClass;

// ─── Label ──────────────────────────────────────────────────────────

Label::~Label() = default;
IClassInfo* Label::Class = &g_LabelClass;

HRESULT Label::ReLayout()
{
    return S_OK;
}

HRESULT Label::OnPaint(HDC hdc, RECT const* rcPaint)
{
    Value* content = GetValue(ContentProp);
    if (!content || content->GetType() != Value::String)
        return S_OK;

    SetBkMode(hdc, TRANSPARENT);

    UINT align = DT_LEFT;
    Value* alignVal = GetValue(ContentAlignProp);
    if (alignVal && alignVal->GetType() == Value::Int)
    {
        int a = alignVal->GetInt();
        if (a == CONTENTALIGN_CENTER)
            align = DT_CENTER;
        else if (a == CONTENTALIGN_RIGHT)
            align = DT_RIGHT;
    }

    RECT textRect = *rcPaint;
    InflateRect(&textRect, -2, -1);
    DrawText(hdc, content->GetString(), -1, &textRect, align | DT_WORDBREAK | DT_VCENTER);

    return S_OK;
}

// ─── Edit ───────────────────────────────────────────────────────────

Edit::~Edit() = default;
IClassInfo* Edit::Class = &g_EditClass;
EventInfo* Edit::LostFocus = &g_LostFocusEvent;
EventInfo* Edit::Change = &g_ChangeEvent;

HRESULT Edit::OnPaint(HDC hdc, RECT const* rcPaint)
{
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, rcPaint, whiteBrush);
    DeleteObject(whiteBrush);

    DrawEdge(hdc, const_cast<RECT*>(rcPaint), EDGE_SUNKEN, BF_RECT);

    Value* content = GetValue(ContentProp);
    if (content && content->GetType() == Value::String)
    {
        SetBkMode(hdc, TRANSPARENT);
        RECT textRect = *rcPaint;
        InflateRect(&textRect, -3, -2);
        DrawText(hdc, content->GetString(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    Value* focused = GetValue(KeyFocusedProp);
    if (focused && focused->GetType() == Value::Bool && focused->GetBool())
    {
        HPEN caretPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        HGDIOBJ oldPen = SelectObject(hdc, caretPen);
        int caretX = rcPaint->left + 5;
        MoveToEx(hdc, caretX, rcPaint->top + 2, nullptr);
        LineTo(hdc, caretX, rcPaint->bottom - 2);
        SelectObject(hdc, oldPen);
        DeleteObject(caretPen);
    }

    return S_OK;
}

// ─── Checkbox ───────────────────────────────────────────────────────

Checkbox::~Checkbox() = default;
IClassInfo* Checkbox::Class = &g_CheckboxClass;
PropertyInfo* Checkbox::CheckedProp = nullptr;
PropertyInfo* Checkbox::UpdateStateProp = nullptr;

HRESULT Checkbox::OnPaint(HDC hdc, RECT const* rcPaint)
{
    int boxSize = 13;
    int boxY = rcPaint->top + ((rcPaint->bottom - rcPaint->top) - boxSize) / 2;
    RECT boxRect = {rcPaint->left + 2, boxY, rcPaint->left + 2 + boxSize, boxY + boxSize};

    Value* checked = GetValue(CheckedProp);
    bool isChecked = checked && checked->GetType() == Value::Bool && checked->GetBool();

    HBRUSH boxBrush = CreateSolidBrush(isChecked ? RGB(0, 102, 204) : RGB(255, 255, 255));
    FillRect(hdc, &boxRect, boxBrush);
    DeleteObject(boxBrush);

    HPEN boxPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HGDIOBJ oldPen = SelectObject(hdc, boxPen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, boxRect.left, boxRect.top, boxRect.right, boxRect.bottom);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(boxPen);

    if (isChecked)
    {
        HPEN checkPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        oldPen = SelectObject(hdc, checkPen);
        MoveToEx(hdc, boxRect.left + 2, boxRect.top + 7, nullptr);
        LineTo(hdc, boxRect.left + 5, boxRect.top + 10);
        LineTo(hdc, boxRect.left + 11, boxRect.top + 3);
        SelectObject(hdc, oldPen);
        DeleteObject(checkPen);
    }

    Value* content = GetValue(ContentProp);
    if (content && content->GetType() == Value::String)
    {
        SetBkMode(hdc, TRANSPARENT);
        RECT textRect = {boxRect.right + 4, rcPaint->top, rcPaint->right - 2, rcPaint->bottom};
        DrawText(hdc, content->GetString(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    return S_OK;
}

// ─── ScrollBar ──────────────────────────────────────────────────────

ScrollBar::~ScrollBar() = default;
IClassInfo* ScrollBar::Class = &g_ScrollBarClass;
PropertyInfo* ScrollBar::LineProp = nullptr;
PropertyInfo* ScrollBar::MaximumProp = nullptr;
PropertyInfo* ScrollBar::PositionProp = nullptr;

HRESULT ScrollBar::OnPaint(HDC hdc, RECT const* rcPaint)
{
    HBRUSH bgBrush = CreateSolidBrush(GetSysColor(COLOR_SCROLLBAR));
    FillRect(hdc, rcPaint, bgBrush);
    DeleteObject(bgBrush);

    int width = rcPaint->right - rcPaint->left;
    int height = rcPaint->bottom - rcPaint->top;
    int arrowSize = (width < height ? width : height) / 3;

    RECT upArrow = {rcPaint->left, rcPaint->top, rcPaint->left + width, rcPaint->top + arrowSize};
    RECT downArrow = {rcPaint->left, rcPaint->bottom - arrowSize, rcPaint->left + width, rcPaint->bottom};

    HBRUSH arrowBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hdc, &upArrow, arrowBrush);
    FillRect(hdc, &downArrow, arrowBrush);
    DeleteObject(arrowBrush);

    int midUpX = (upArrow.left + upArrow.right) / 2;
    POINT upPts[3];
    upPts[0].x = midUpX;       upPts[0].y = upArrow.top + 2;
    upPts[1].x = midUpX - 5;  upPts[1].y = upArrow.bottom - 2;
    upPts[2].x = midUpX + 5;  upPts[2].y = upArrow.bottom - 2;

    int midDownX = (downArrow.left + downArrow.right) / 2;
    POINT downPts[3];
    downPts[0].x = midDownX;       downPts[0].y = downArrow.bottom - 2;
    downPts[1].x = midDownX - 5;  downPts[1].y = downArrow.top + 2;
    downPts[2].x = midDownX + 5;  downPts[2].y = downArrow.top + 2;

    HPEN arrowPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNTEXT));
    HGDIOBJ oldPen = SelectObject(hdc, arrowPen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Polygon(hdc, upPts, 3);
    Polygon(hdc, downPts, 3);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(arrowPen);

    int trackTop = upArrow.bottom;
    int trackBottom = downArrow.top;
    int trackHeight = trackBottom - trackTop;

    Value* maxVal = GetValue(MaximumProp);
    Value* posVal = GetValue(PositionProp);
    int maximum = 100;
    int position = 0;
    if (maxVal && maxVal->GetType() == Value::Int)
        maximum = maxVal->GetInt();
    if (posVal && posVal->GetType() == Value::Int)
        position = posVal->GetInt();
    if (maximum <= 0) maximum = 1;
    if (position < 0) position = 0;
    if (position > maximum) position = maximum;

    int thumbMinHeight = 15;
    int thumbHeight = trackHeight * 10 / (maximum + 10);
    if (thumbHeight < thumbMinHeight) thumbHeight = thumbMinHeight;
    if (thumbHeight > trackHeight) thumbHeight = trackHeight;

    int thumbTop = trackTop;
    if (trackHeight > thumbHeight)
        thumbTop = trackTop + (trackHeight - thumbHeight) * position / maximum;

    RECT thumbRect = {rcPaint->left + 2, thumbTop, rcPaint->left + width - 2, thumbTop + thumbHeight};

    HBRUSH thumbBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hdc, &thumbRect, thumbBrush);
    DeleteObject(thumbBrush);

    DrawEdge(hdc, &thumbRect, EDGE_RAISED, BF_RECT);

    return S_OK;
}

// ─── Slider ─────────────────────────────────────────────────────────

Slider::~Slider() = default;
IClassInfo* Slider::Class = &g_SliderClass;
PropertyInfo* Slider::PositionProp = nullptr;
PropertyInfo* Slider::MinimumProp = nullptr;
PropertyInfo* Slider::MaximumProp = nullptr;

HRESULT Slider::OnPaint(HDC hdc, RECT const* rcPaint)
{
    int midY = (rcPaint->top + rcPaint->bottom) / 2;
    int leftX = rcPaint->left + 6;
    int rightX = rcPaint->right - 6;

    HPEN trackPen = CreatePen(PS_SOLID, 3, RGB(100, 100, 100));
    HGDIOBJ oldPen = SelectObject(hdc, trackPen);
    MoveToEx(hdc, leftX, midY, nullptr);
    LineTo(hdc, rightX, midY);
    SelectObject(hdc, oldPen);
    DeleteObject(trackPen);

    Value* maxVal = GetValue(MaximumProp);
    Value* minVal = GetValue(MinimumProp);
    Value* posVal = GetValue(PositionProp);
    int maximum = 100;
    int minimum = 0;
    int position = 0;
    if (maxVal && maxVal->GetType() == Value::Int)
        maximum = maxVal->GetInt();
    if (minVal && minVal->GetType() == Value::Int)
        minimum = minVal->GetInt();
    if (posVal && posVal->GetType() == Value::Int)
        position = posVal->GetInt();
    int range = maximum - minimum;
    if (range <= 0) range = 1;
    if (position < minimum) position = minimum;
    if (position > maximum) position = maximum;

    int thumbPos = leftX + (rightX - leftX) * (position - minimum) / range;

    int thumbRadius = 6;
    HBRUSH thumbBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    HGDIOBJ oldBrush = SelectObject(hdc, thumbBrush);
    HPEN thumbPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    oldPen = SelectObject(hdc, thumbPen);

    Ellipse(hdc, thumbPos - thumbRadius, midY - thumbRadius,
            thumbPos + thumbRadius, midY + thumbRadius);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(thumbPen);
    DeleteObject(thumbBrush);

    return S_OK;
}

// ─── Combobox ───────────────────────────────────────────────────────

Combobox::~Combobox() = default;
IClassInfo* Combobox::Class = &g_ComboboxClass;
PropertyInfo* Combobox::ItemsProp = nullptr;
PropertyInfo* Combobox::SelectionProp = nullptr;
EventInfo* Combobox::SelectionChange = &g_SelectionChangeEvent;

// ─── Thumb ──────────────────────────────────────────────────────────

Thumb::~Thumb() = default;
IClassInfo* Thumb::Class = &g_ThumbClass;

HRESULT Thumb::OnPaint(HDC hdc, RECT const* rcPaint)
{
    DrawEdge(hdc, const_cast<RECT*>(rcPaint), EDGE_RAISED, BF_RECT);
    return S_OK;
}

// ─── Hyperlink ──────────────────────────────────────────────────────

Hyperlink::~Hyperlink() = default;
IClassInfo* Hyperlink::Class = &g_HyperlinkClass;
PropertyInfo* Hyperlink::UrlProp = nullptr;
EventInfo* Hyperlink::Navigate = &g_NavigateEvent;

// ─── WLEditT ────────────────────────────────────────────────────────

WLEditT::~WLEditT() = default;
IClassInfo* WLEditT::Class = &g_WLEditTClass;

HRESULT WLEditT::GetContentSize(SIZE* pSize)
{
    if (!pSize) return E_POINTER;
    return E_NOTIMPL;
}

HRESULT WLEditT::GetPosFromChar(UINT charIndex, POINT* pPoint)
{
    UNREFERENCED_PARAMETER(charIndex);
    if (!pPoint) return E_POINTER;
    return E_NOTIMPL;
}

// ─── Selector ───────────────────────────────────────────────────────

Selector::~Selector() = default;
IClassInfo* Selector::Class = &g_SelectorClass;
PropertyInfo* Selector::SelectionProp = nullptr;
EventInfo* Selector::SelectionChange = &g_SelectionChangeEvent;

} // namespace DirectUI
