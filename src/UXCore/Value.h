#pragma once
#include <windows.h>
#include <ole2.h>

namespace DirectUI {

class Element;
class Layout;

class __declspec(dllexport) Value {
public:
    enum Type : int {
        Null = 0,
        Bool,
        Int,
        Float,
        Double,
        String,
        Color,
        Size,
        Point,
        Rect,
        GUID,
        LayoutPtr,
        ElementRef,
        ElementList,
        Unknown,
        ValueList,
        Graphic,
        Atom,
        Cursor,
        RLE,
        Animation,
        DFC,
        DTB,
        Colorize
    };

    Value();
    explicit Value(Type type);
    ~Value();

    ULONG AddRef();
    ULONG Release();
    Type GetType() const;
    void _ZeroRelease();

    // Accessors
    int GetInt() const;
    bool GetBool() const;
    float GetFloat() const;
    double GetDouble() const;
    const wchar_t* GetString() const;
    ULONG GetColor() const;
    SIZE GetSize() const;
    POINT GetPoint() const;
    RECT GetRect() const;
    void GetGUID(void* guid) const;
    Layout* GetLayout() const;
    Element* GetElementRef() const;
    IUnknown* GetUnknown() const;

    // Static factory methods
    static Value* CreateInt(int value);
    static Value* CreateBool(bool value);
    static Value* CreateString(const wchar_t* value);
    static Value* CreateFloat(float value);
    static Value* CreateDouble(double value);
    static Value* CreateColor(ULONG value);
    static Value* CreateSize(SIZE value);
    static Value* CreatePoint(POINT value);
    static Value* CreateRect(RECT value);
    static Value* CreateLayout(Layout* value);
    static Value* CreateElementRef(Element** ptr);
    static Value* CreateUnknown(IUnknown* value);
    static Value* CreateValueList(Value** values, int count);
    static Value* CreateGraphic(void* graphic);

    // Static sentinel constants
    static Value* pvNull;
    static Value* pvUnset;
    static Value* pvUnavailable;
    static Value* pvBoolTrue;
    static Value* pvBoolFalse;
    static Value* pvIntZero;
    static Value* pvStringNull;
    static Value* pvStringEmpty;
    static Value* pvElementNull;
    static Value* pvLayoutNull;
    static Value* pvPointZero;
    static Value* pvRectZero;
    static Value* pvSizeZero;
    static Value* pvFloatZero;
    static Value* pvDoubleZero;
    static Value* pvColorTrans;

private:
    void FreeData();

    LONG m_refCount = 1;
    Type m_type = Null;
    bool m_ownsString = false;

    union Data {
        bool m_bool;
        int m_int;
        float m_float;
        double m_double;
        wchar_t* m_string;
        ULONG m_color;
        SIZE m_size;
        POINT m_point;
        RECT m_rect;
        unsigned char m_guid[16];
        Layout* m_layout;
        Element* m_elementRef;
        IUnknown* m_unknown;
        void* m_ptr;
        struct {
            Value** items;
            int count;
        } m_valueList;
    } m_data;

    void SetData(const Data& data);

    friend struct SentinelInit;
};

} // namespace DirectUI
