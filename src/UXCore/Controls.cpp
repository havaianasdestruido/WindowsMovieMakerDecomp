#include "Controls.h"

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
EventInfo* Button::Click = &g_ClickEvent;

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

// ─── Edit ───────────────────────────────────────────────────────────

Edit::~Edit() = default;
IClassInfo* Edit::Class = &g_EditClass;
EventInfo* Edit::LostFocus = &g_LostFocusEvent;
EventInfo* Edit::Change = &g_ChangeEvent;

// ─── Checkbox ───────────────────────────────────────────────────────

Checkbox::~Checkbox() = default;
IClassInfo* Checkbox::Class = &g_CheckboxClass;
PropertyInfo* Checkbox::CheckedProp = nullptr;
PropertyInfo* Checkbox::UpdateStateProp = nullptr;

// ─── ScrollBar ──────────────────────────────────────────────────────

ScrollBar::~ScrollBar() = default;
IClassInfo* ScrollBar::Class = &g_ScrollBarClass;
PropertyInfo* ScrollBar::LineProp = nullptr;
PropertyInfo* ScrollBar::MaximumProp = nullptr;
PropertyInfo* ScrollBar::PositionProp = nullptr;

// ─── Slider ─────────────────────────────────────────────────────────

Slider::~Slider() = default;
IClassInfo* Slider::Class = &g_SliderClass;
PropertyInfo* Slider::PositionProp = nullptr;
PropertyInfo* Slider::MinimumProp = nullptr;
PropertyInfo* Slider::MaximumProp = nullptr;

// ─── Combobox ───────────────────────────────────────────────────────

Combobox::~Combobox() = default;
IClassInfo* Combobox::Class = &g_ComboboxClass;
PropertyInfo* Combobox::ItemsProp = nullptr;
PropertyInfo* Combobox::SelectionProp = nullptr;
EventInfo* Combobox::SelectionChange = &g_SelectionChangeEvent;

// ─── Thumb ──────────────────────────────────────────────────────────

Thumb::~Thumb() = default;
IClassInfo* Thumb::Class = &g_ThumbClass;

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
