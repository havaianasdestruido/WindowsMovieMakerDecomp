#pragma once
#include "Element.h"

#define CONTENTALIGN_LEFT   0
#define CONTENTALIGN_CENTER 1
#define CONTENTALIGN_RIGHT  2

#pragma warning(push)
#pragma warning(disable: 4251)

namespace DirectUI {

struct IClassInfo {};

class __declspec(dllexport) ControlBase : public Element {
public:
    virtual ~ControlBase();

    HRESULT AddHandler(EventInfo* event, EventCallback callback);
    HRESULT RemoveHandler(EventInfo* event, EventCallback callback);
    HRESULT FireEvent(EventInfo* event, int numArgs, Value** args);

protected:
    struct HandlerEntry {
        EventInfo* event;
        EventCallback callback;
    };
    std::vector<HandlerEntry> m_handlers;
};

class __declspec(dllexport) Button : public ControlBase {
public:
    virtual ~Button();

    HRESULT OnPaint(HDC hdc, RECT const* rcPaint) override;

    static IClassInfo* Class;
    static PropertyInfo* PressedProp;
    static PropertyInfo* FontSizeProp;
    static EventInfo* Click;
};

class __declspec(dllexport) ButtonText : public ControlBase {
public:
    virtual ~ButtonText();

    static IClassInfo* Class;
};

class __declspec(dllexport) Label : public ControlBase {
public:
    virtual ~Label();

    HRESULT OnPaint(HDC hdc, RECT const* rcPaint) override;

    static IClassInfo* Class;
    HRESULT ReLayout();
};

class __declspec(dllexport) Edit : public ControlBase {
public:
    virtual ~Edit();

    HRESULT OnPaint(HDC hdc, RECT const* rcPaint) override;

    static IClassInfo* Class;
    static EventInfo* LostFocus;
    static EventInfo* Change;
};

class __declspec(dllexport) Checkbox : public ControlBase {
public:
    virtual ~Checkbox();

    HRESULT OnPaint(HDC hdc, RECT const* rcPaint) override;

    static IClassInfo* Class;
    static PropertyInfo* CheckedProp;
    static PropertyInfo* UpdateStateProp;
};

class __declspec(dllexport) ScrollBar : public ControlBase {
public:
    virtual ~ScrollBar();

    HRESULT OnPaint(HDC hdc, RECT const* rcPaint) override;

    static IClassInfo* Class;
    static PropertyInfo* LineProp;
    static PropertyInfo* MaximumProp;
    static PropertyInfo* PositionProp;
};

class __declspec(dllexport) Slider : public ControlBase {
public:
    virtual ~Slider();

    HRESULT OnPaint(HDC hdc, RECT const* rcPaint) override;

    static IClassInfo* Class;
    static PropertyInfo* PositionProp;
    static PropertyInfo* MinimumProp;
    static PropertyInfo* MaximumProp;
};

class __declspec(dllexport) Combobox : public ControlBase {
public:
    virtual ~Combobox();

    static IClassInfo* Class;
    static PropertyInfo* ItemsProp;
    static PropertyInfo* SelectionProp;
    static EventInfo* SelectionChange;
};

class __declspec(dllexport) Thumb : public ControlBase {
public:
    virtual ~Thumb();

    HRESULT OnPaint(HDC hdc, RECT const* rcPaint) override;

    static IClassInfo* Class;
};

class __declspec(dllexport) Hyperlink : public ControlBase {
public:
    virtual ~Hyperlink();

    static IClassInfo* Class;
    static PropertyInfo* UrlProp;
    static EventInfo* Navigate;
};

class __declspec(dllexport) WLEditT : public ControlBase {
public:
    virtual ~WLEditT();

    static IClassInfo* Class;
    HRESULT GetContentSize(SIZE* pSize);
    HRESULT GetPosFromChar(UINT charIndex, POINT* pPoint);
};

class __declspec(dllexport) Selector : public ControlBase {
public:
    virtual ~Selector();

    static IClassInfo* Class;
    static PropertyInfo* SelectionProp;
    static EventInfo* SelectionChange;
};

} // namespace DirectUI

#pragma warning(pop)
