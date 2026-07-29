#pragma once
#include <windows.h>
#include <ole2.h>

namespace DirectUI {

class Element;

struct PropertyInfo;

class __declspec(dllexport) Layout {
public:
    virtual ~Layout();
    virtual HRESULT _DoLayout(Element* element, SIZE layoutSize) = 0;
    virtual SIZE _GetDesiredSize(Element* element, SIZE availableSize) = 0;
};

class __declspec(dllexport) FillLayout : public Layout {
public:
    static FillLayout* Create();

    HRESULT _DoLayout(Element* element, SIZE layoutSize) override;
    SIZE _GetDesiredSize(Element* element, SIZE availableSize) override;
};

class __declspec(dllexport) VirtualLayout : public Layout {
public:
    static PropertyInfo* TemplateSizeProp;
    static PropertyInfo* DataSourceProp;
    static PropertyInfo* AnimationsEnabledProp;
    static PropertyInfo* LayoutModeInterfaceProp;
    static PropertyInfo* TemplateSourceProp;

    HRESULT _DoLayout(Element* element, SIZE layoutSize) override;
    SIZE _GetDesiredSize(Element* element, SIZE availableSize) override;

    HRESULT SetKeyFocus(Element* element, Element* target);
    Element* GetTemplate(Element* element);
    HRESULT SetNextKeyFocus(Element* element, Element* next);
    int GetTemplateIndex(Element* element);
    Element* GetLastKeyFocusedTemplate(Element* element);
};

class __declspec(dllexport) BorderLayout : public Layout {
public:
    HRESULT _DoLayout(Element* element, SIZE layoutSize) override;
    SIZE _GetDesiredSize(Element* element, SIZE availableSize) override;
};

class __declspec(dllexport) GridLayout : public Layout {
public:
    HRESULT _DoLayout(Element* element, SIZE layoutSize) override;
    SIZE _GetDesiredSize(Element* element, SIZE availableSize) override;
};

class __declspec(dllexport) RowLayout : public Layout {
public:
    HRESULT _DoLayout(Element* element, SIZE layoutSize) override;
    SIZE _GetDesiredSize(Element* element, SIZE availableSize) override;
};

class __declspec(dllexport) FlowLayout : public Layout {
public:
    HRESULT _DoLayout(Element* element, SIZE layoutSize) override;
    SIZE _GetDesiredSize(Element* element, SIZE availableSize) override;
};

class __declspec(dllexport) NineGridLayout : public Layout {
public:
    HRESULT _DoLayout(Element* element, SIZE layoutSize) override;
    SIZE _GetDesiredSize(Element* element, SIZE availableSize) override;
};

} // namespace DirectUI
