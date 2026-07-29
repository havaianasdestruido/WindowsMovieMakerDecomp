#pragma once
#include "Element.h"

#pragma warning(push)
#pragma warning(disable: 4251)

namespace DirectUI {

struct IClassInfo {};
struct EventInfo {
    const wchar_t* name;
};

typedef HRESULT (CALLBACK *EventCallback)(Element* sender, Value** args, int numArgs);

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

    static IClassInfo* Class;
    static PropertyInfo* PressedProp;
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

    static IClassInfo* Class;
    HRESULT ReLayout();
};

class __declspec(dllexport) Edit : public ControlBase {
public:
    virtual ~Edit();

    static IClassInfo* Class;
    static EventInfo* LostFocus;
    static EventInfo* Change;
};

class __declspec(dllexport) Checkbox : public ControlBase {
public:
    virtual ~Checkbox();

    static IClassInfo* Class;
    static PropertyInfo* CheckedProp;
    static PropertyInfo* UpdateStateProp;
};

class __declspec(dllexport) ScrollBar : public ControlBase {
public:
    virtual ~ScrollBar();

    static IClassInfo* Class;
    static PropertyInfo* LineProp;
    static PropertyInfo* MaximumProp;
    static PropertyInfo* PositionProp;
};

class __declspec(dllexport) Slider : public ControlBase {
public:
    virtual ~Slider();

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
