#include "Value.h"
#include <cstdlib>

namespace DirectUI {

// Sentinel value storage
static Value g_pvNull(Value::Null);
static Value g_pvUnset(Value::Null);
static Value g_pvUnavailable(Value::Null);
static Value g_pvBoolTrue(Value::Bool);
static Value g_pvBoolFalse(Value::Bool);
static Value g_pvIntZero(Value::Int);
static Value g_pvStringNull(Value::String);
static Value g_pvStringEmpty(Value::String);
static Value g_pvElementNull(Value::Null);
static Value g_pvLayoutNull(Value::Null);
static Value g_pvPointZero(Value::Point);
static Value g_pvRectZero(Value::Rect);
static Value g_pvSizeZero(Value::Size);
static Value g_pvFloatZero(Value::Float);
static Value g_pvDoubleZero(Value::Double);
static Value g_pvColorTrans(Value::Color);

Value* Value::pvNull = &g_pvNull;
Value* Value::pvUnset = &g_pvUnset;
Value* Value::pvUnavailable = &g_pvUnavailable;
Value* Value::pvBoolTrue = &g_pvBoolTrue;
Value* Value::pvBoolFalse = &g_pvBoolFalse;
Value* Value::pvIntZero = &g_pvIntZero;
Value* Value::pvStringNull = &g_pvStringNull;
Value* Value::pvStringEmpty = &g_pvStringEmpty;
Value* Value::pvElementNull = &g_pvElementNull;
Value* Value::pvLayoutNull = &g_pvLayoutNull;
Value* Value::pvPointZero = &g_pvPointZero;
Value* Value::pvRectZero = &g_pvRectZero;
Value* Value::pvSizeZero = &g_pvSizeZero;
Value* Value::pvFloatZero = &g_pvFloatZero;
Value* Value::pvDoubleZero = &g_pvDoubleZero;
Value* Value::pvColorTrans = &g_pvColorTrans;

// --- Sentinel initializers (called once during static init) ---
struct SentinelInit {
    SentinelInit() {
        g_pvBoolTrue.m_ownsString = false;
        g_pvBoolTrue.m_data.m_bool = true;
        g_pvBoolFalse.m_ownsString = false;
        g_pvBoolFalse.m_data.m_bool = false;
        g_pvIntZero.m_ownsString = false;
        g_pvIntZero.m_data.m_int = 0;
        g_pvStringNull.m_ownsString = false;
        g_pvStringNull.m_data.m_string = nullptr;
        g_pvStringEmpty.m_ownsString = false;
        g_pvStringEmpty.m_data.m_string = L"";
        g_pvPointZero.m_ownsString = false;
        g_pvPointZero.m_data.m_point = POINT{0, 0};
        g_pvRectZero.m_ownsString = false;
        g_pvRectZero.m_data.m_rect = RECT{0, 0, 0, 0};
        g_pvSizeZero.m_ownsString = false;
        g_pvSizeZero.m_data.m_size = SIZE{0, 0};
        g_pvFloatZero.m_ownsString = false;
        g_pvFloatZero.m_data.m_float = 0.0f;
        g_pvDoubleZero.m_ownsString = false;
        g_pvDoubleZero.m_data.m_double = 0.0;
        g_pvColorTrans.m_ownsString = false;
        g_pvColorTrans.m_data.m_color = 0x00000000;
    }
};
static SentinelInit g_sentinelInit;

// Constructor / Destructor
Value::Value()
    : m_refCount(1), m_type(Null)
{
    memset(&m_data, 0, sizeof(m_data));
}

Value::Value(Type type)
    : m_refCount(1), m_type(type)
{
    memset(&m_data, 0, sizeof(m_data));
}

Value::~Value()
{
    FreeData();
}

void Value::FreeData()
{
    if (m_type == String && m_ownsString && m_data.m_string)
    {
        free(const_cast<wchar_t*>(m_data.m_string));
        m_data.m_string = nullptr;
        m_ownsString = false;
    }
    if (m_type == ValueList && m_data.m_valueList.items)
    {
        free(m_data.m_valueList.items);
        m_data.m_valueList.items = nullptr;
    }
}

// Reference counting
ULONG Value::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG Value::Release()
{
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0)
        delete this;
    return count;
}

void Value::_ZeroRelease()
{
    FreeData();
    memset(&m_data, 0, sizeof(m_data));
    m_type = Null;
    m_refCount = 0;
}

Element* Value::GetElementRef() const
{
    return m_data.m_elementRef;
}

void Value::GetGUID(void* guidBuf) const
{
    if (guidBuf)
        memcpy(guidBuf, m_data.m_guid, 16);
}

// Accessors
Value::Type Value::GetType() const
{
    return m_type;
}

int Value::GetInt() const
{
    return m_data.m_int;
}

bool Value::GetBool() const
{
    return m_data.m_bool;
}

float Value::GetFloat() const
{
    return m_data.m_float;
}

double Value::GetDouble() const
{
    return m_data.m_double;
}

const wchar_t* Value::GetString() const
{
    return m_data.m_string;
}

ULONG Value::GetColor() const
{
    return m_data.m_color;
}

SIZE Value::GetSize() const
{
    return m_data.m_size;
}

POINT Value::GetPoint() const
{
    return m_data.m_point;
}

RECT Value::GetRect() const
{
    return m_data.m_rect;
}

Layout* Value::GetLayout() const
{
    return m_data.m_layout;
}

IUnknown* Value::GetUnknown() const
{
    return m_data.m_unknown;
}

void Value::SetData(const Data& data)
{
    m_data = data;
}

// Factory methods
Value* Value::CreateInt(int value)
{
    Value* v = new Value(Int);
    v->m_data.m_int = value;
    return v;
}

Value* Value::CreateBool(bool value)
{
    return value ? pvBoolTrue : pvBoolFalse;
}

Value* Value::CreateString(const wchar_t* value)
{
    if (!value) return pvStringNull;
    if (value[0] == L'\0') return pvStringEmpty;
    Value* v = new Value(String);
    size_t len = wcslen(value) + 1;
    v->m_data.m_string = static_cast<wchar_t*>(malloc(len * sizeof(wchar_t)));
    if (v->m_data.m_string)
    {
        wcscpy_s(const_cast<wchar_t*>(v->m_data.m_string), len, value);
        v->m_ownsString = true;
    }
    return v;
}

Value* Value::CreateFloat(float value)
{
    if (value == 0.0f) return pvFloatZero;
    Value* v = new Value(Float);
    v->m_data.m_float = value;
    return v;
}

Value* Value::CreateDouble(double value)
{
    if (value == 0.0) return pvDoubleZero;
    Value* v = new Value(Double);
    v->m_data.m_double = value;
    return v;
}

Value* Value::CreateColor(ULONG value)
{
    if (value == 0x00000000) return pvColorTrans;
    Value* v = new Value(Color);
    v->m_data.m_color = value;
    return v;
}

Value* Value::CreateSize(SIZE value)
{
    if (value.cx == 0 && value.cy == 0) return pvSizeZero;
    Value* v = new Value(Size);
    v->m_data.m_size = value;
    return v;
}

Value* Value::CreatePoint(POINT value)
{
    if (value.x == 0 && value.y == 0) return pvPointZero;
    Value* v = new Value(Point);
    v->m_data.m_point = value;
    return v;
}

Value* Value::CreateRect(RECT value)
{
    if (value.left == 0 && value.top == 0 && value.right == 0 && value.bottom == 0)
        return pvRectZero;
    Value* v = new Value(Rect);
    v->m_data.m_rect = value;
    return v;
}

Value* Value::CreateLayout(Layout* value)
{
    if (!value) return pvLayoutNull;
    Value* v = new Value(LayoutPtr);
    v->m_data.m_layout = value;
    return v;
}

Value* Value::CreateElementRef(Element** ptr)
{
    if (!ptr) return pvElementNull;
    Value* v = new Value(ElementRef);
    v->m_data.m_elementRef = ptr ? *ptr : nullptr;
    return v;
}

Value* Value::CreateUnknown(IUnknown* value)
{
    if (!value) return pvUnavailable;
    Value* v = new Value(Unknown);
    v->m_data.m_unknown = value;
    if (value) value->AddRef();
    return v;
}

Value* Value::CreateValueList(Value** values, int count)
{
    if (!values || count == 0) return pvNull;
    Value* v = new Value(ValueList);
    v->m_data.m_valueList.count = count;
    v->m_data.m_valueList.items = static_cast<Value**>(malloc(count * sizeof(Value*)));
    if (v->m_data.m_valueList.items)
    {
        for (int i = 0; i < count; i++)
            v->m_data.m_valueList.items[i] = values[i];
    }
    return v;
}

Value* Value::CreateGraphic(void* graphic)
{
    if (!graphic) return pvNull;
    Value* v = new Value(Graphic);
    v->m_data.m_ptr = graphic;
    return v;
}

} // namespace DirectUI
