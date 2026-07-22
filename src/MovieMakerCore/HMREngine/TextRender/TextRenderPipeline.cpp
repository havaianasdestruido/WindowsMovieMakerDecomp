#include "pch.h"
// TextRenderPipeline.cpp - Text rendering task pipeline implementation

#include "TextRenderPipeline.h"
#include <dwrite.h>
#include <d2d1.h>
#include <wincodec.h>

#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace HMREngine
{

// ============================================================================
// TaskFinishNotificationCallback
// ============================================================================
TaskFinishNotificationCallback::TaskFinishNotificationCallback() = default;
TaskFinishNotificationCallback::~TaskFinishNotificationCallback() = default;

// ============================================================================
// TextRenderTask
// ============================================================================
TextRenderTask::TextRenderTask() = default;
TextRenderTask::~TextRenderTask() = default;

void TextRenderTask::Cancel()
{
    if (m_state == TaskState::Pending || m_state == TaskState::Running)
        m_state = TaskState::Cancelled;
}

// ============================================================================
// Helper: Create WIC factory (singleton)
// ============================================================================
static IWICImagingFactory* GetWICFactory()
{
    static CComPtr<IWICImagingFactory> s_factory;
    if (!s_factory)
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory), (void**)&s_factory);
    return s_factory;
}

// ============================================================================
// Helper: Create D3D11 texture + SRV from WIC bitmap pixel data
// ============================================================================
HRESULT TextRenderTask::CreateTextureFromWICBitmap(IWICBitmap* pWicBitmap, UINT texW, UINT texH)
{
    if (!pWicBitmap || !m_device || !m_context)
        return E_INVALIDARG;

    m_resultTexture.Release();
    m_resultSRV.Release();

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = texW;
    td.Height = texH;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_resultTexture);
    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapLock> lock;
    WICRect wicRect = { 0, 0, static_cast<INT>(texW), static_cast<INT>(texH) };
    hr = pWicBitmap->Lock(&wicRect, WICBitmapLockRead, &lock);
    if (FAILED(hr))
        return hr;

    BYTE* data = nullptr;
    UINT bufferSize = 0;
    UINT stride = 0;
    lock->GetDataPointer(&bufferSize, &data);
    lock->GetStride(&stride);

    if (data && bufferSize > 0)
        m_context->UpdateSubresource(m_resultTexture, 0, nullptr, data, stride, 0);

    lock.Release();

    hr = m_device->CreateShaderResourceView(m_resultTexture, nullptr, &m_resultSRV);
    return hr;
}

// ============================================================================
// Helper: Create DWrite factory (singleton)
// ============================================================================
static IDWriteFactory* GetDWriteFactory()
{
    static CComPtr<IDWriteFactory> s_factory;
    if (!s_factory)
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&s_factory));
    return s_factory;
}

// ============================================================================
// Helper: Create D2D factory (singleton)
// ============================================================================
static ID2D1Factory* GetD2DFactory()
{
    static CComPtr<ID2D1Factory> s_factory;
    if (!s_factory)
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), reinterpret_cast<void**>(&s_factory));
    return s_factory;
}

// ============================================================================
// PartialTextRenderTask
// ============================================================================
PartialTextRenderTask::PartialTextRenderTask() = default;
PartialTextRenderTask::~PartialTextRenderTask() = default;

HRESULT PartialTextRenderTask::Execute()
{
    m_state = TaskState::Running;

    IDWriteFactory* dwFactory = GetDWriteFactory();
    ID2D1Factory* d2dFactory = GetD2DFactory();
    if (!dwFactory || !d2dFactory) { m_state = TaskState::Failed; return E_FAIL; }

    CComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = dwFactory->CreateTextFormat(m_fontFamily.c_str(), nullptr,
        m_bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        m_italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, m_fontSize, L"en-us", &textFormat);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    CComPtr<IDWriteTextLayout> textLayout;
    hr = dwFactory->CreateTextLayout(m_text.c_str(), static_cast<UINT32>(m_text.length()),
        textFormat, 4096.0f, 1024.0f, &textLayout);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    DWRITE_TEXT_METRICS metrics;
    textLayout->GetMetrics(&metrics);

    UINT texW = static_cast<UINT>(ceilf(metrics.width) + 4);
    UINT texH = static_cast<UINT>(ceilf(metrics.height) + 4);
    if (texW < 4) texW = 4;
    if (texH < 4) texH = 4;

    m_width = texW;
    m_height = texH;

    CComPtr<IWICImagingFactory> wicFactory = GetWICFactory();
    if (!wicFactory) { m_state = TaskState::Failed; return E_FAIL; }

    CComPtr<IWICBitmap> wicBitmap;
    hr = wicFactory->CreateBitmap(texW, texH, GUID_WICPixelFormat32bppBGRA,
        WICBitmapCacheOnDemand, &wicBitmap);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    CComPtr<ID2D1RenderTarget> rt;
    hr = d2dFactory->CreateWicBitmapRenderTarget(wicBitmap, rtProps, &rt);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    rt->BeginDraw();

    D2D1_COLOR_F bgColor = D2D1::ColorF(0, 0, 0, 0);
    rt->Clear(bgColor);

    D2D1_COLOR_F fgColor = D2D1::ColorF(m_color.x, m_color.y, m_color.z, m_color.w);
    CComPtr<ID2D1SolidColorBrush> brush;
    rt->CreateSolidColorBrush(fgColor, &brush);

    rt->DrawTextLayout(D2D1::Point2F(0, 0), textLayout, brush);

    hr = rt->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        m_state = TaskState::Failed;
        return hr;
    }

    CComPtr<IWICBitmapLock> lock;
    WICRect wicRect = { 0, 0, static_cast<INT>(texW), static_cast<INT>(texH) };
    hr = wicBitmap->Lock(&wicRect, WICBitmapLockRead, &lock);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    BYTE* data = nullptr;
    UINT bufferSize = 0;
    UINT stride = 0;
    lock->GetDataPointer(&bufferSize, &data);
    lock->GetStride(&stride);

    if (!m_device) { m_state = TaskState::Failed; return E_POINTER; }

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = texW;
    td.Height = texH;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    std::vector<BYTE> pixelData(texW * texH * 4);
    hr = wicBitmap->CopyPixels(nullptr, texW * 4, texW * texH * 4, pixelData.data());
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixelData.data();
    initData.SysMemPitch = texW * 4;

    m_resultTexture.Release();
    m_resultSRV.Release();

    hr = m_device->CreateTexture2D(&td, &initData, &m_resultTexture);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    hr = m_device->CreateShaderResourceView(m_resultTexture, nullptr, &m_resultSRV);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    m_state = TaskState::Completed;
    if (m_callback) m_callback->OnTaskFinished(this);

    return S_OK;
}

// ============================================================================
// PartialTextPreRenderTask
// ============================================================================
PartialTextPreRenderTask::PartialTextPreRenderTask() = default;
PartialTextPreRenderTask::~PartialTextPreRenderTask() = default;

HRESULT PartialTextPreRenderTask::Execute()
{
    m_state = TaskState::Running;

    IDWriteFactory* dwFactory = GetDWriteFactory();
    if (!dwFactory) { m_state = TaskState::Failed; return E_FAIL; }

    CComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = dwFactory->CreateTextFormat(m_fontFamily.c_str(), nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, m_fontSize, L"en-us", &textFormat);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    CComPtr<IDWriteTextLayout> textLayout;
    hr = dwFactory->CreateTextLayout(m_text.c_str(), static_cast<UINT32>(m_text.length()),
        textFormat, 4096.0f, 4096.0f, &textLayout);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    DWRITE_TEXT_METRICS metrics;
    textLayout->GetMetrics(&metrics);

    m_measuredWidth = metrics.width;
    m_measuredHeight = metrics.height;
    m_width = static_cast<UINT>(ceilf(metrics.width));
    m_height = static_cast<UINT>(ceilf(metrics.height));

    m_state = TaskState::Completed;
    if (m_callback) m_callback->OnTaskFinished(this);
    return S_OK;
}

// ============================================================================
// TextBitmapRenderTask
// ============================================================================
TextBitmapRenderTask::TextBitmapRenderTask() = default;
TextBitmapRenderTask::~TextBitmapRenderTask() = default;

HRESULT TextBitmapRenderTask::Execute()
{
    m_state = TaskState::Running;

    IDWriteFactory* dwFactory = GetDWriteFactory();
    ID2D1Factory* d2dFactory = GetD2DFactory();
    if (!dwFactory || !d2dFactory) { m_state = TaskState::Failed; return E_FAIL; }

    CComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = dwFactory->CreateTextFormat(m_fontFamily.c_str(), nullptr,
        m_bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        m_italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, m_fontSize, L"en-us", &textFormat);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    CComPtr<IDWriteTextLayout> textLayout;
    hr = dwFactory->CreateTextLayout(m_text.c_str(), static_cast<UINT32>(m_text.length()),
        textFormat, 4096.0f, 1024.0f, &textLayout);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    DWRITE_TEXT_METRICS metrics;
    textLayout->GetMetrics(&metrics);

    m_width = static_cast<UINT>(ceilf(metrics.width) + 4);
    m_height = static_cast<UINT>(ceilf(metrics.height) + 4);
    if (m_width < 4) m_width = 4;
    if (m_height < 4) m_height = 4;

    CComPtr<IWICImagingFactory> wicFactory = GetWICFactory();
    if (!wicFactory) { m_state = TaskState::Failed; return E_FAIL; }

    CComPtr<IWICBitmap> wicBitmap;
    hr = wicFactory->CreateBitmap(m_width, m_height, GUID_WICPixelFormat32bppBGRA,
        WICBitmapCacheOnDemand, &wicBitmap);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    CComPtr<ID2D1RenderTarget> rt;
    hr = d2dFactory->CreateWicBitmapRenderTarget(wicBitmap, rtProps, &rt);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    rt->BeginDraw();

    D2D1_COLOR_F bgColor = D2D1::ColorF(m_backgroundColor.x, m_backgroundColor.y,
        m_backgroundColor.z, m_backgroundColor.w);
    rt->Clear(bgColor);

    D2D1_COLOR_F fgColor = D2D1::ColorF(m_color.x, m_color.y, m_color.z, m_color.w);
    CComPtr<ID2D1SolidColorBrush> brush;
    rt->CreateSolidColorBrush(fgColor, &brush);

    rt->DrawTextLayout(D2D1::Point2F(0, 0), textLayout, brush);

    hr = rt->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) { m_state = TaskState::Failed; return hr; }

    CComPtr<IWICBitmapLock> lock;
    WICRect wicRect = { 0, 0, static_cast<INT>(m_width), static_cast<INT>(m_height) };
    hr = wicBitmap->Lock(&wicRect, WICBitmapLockRead, &lock);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    BYTE* data = nullptr;
    UINT bufferSize = 0;
    UINT stride = 0;
    lock->GetDataPointer(&bufferSize, &data);
    lock->GetStride(&stride);

    m_bitmapData.assign(data, data + bufferSize);

    m_state = TaskState::Completed;
    if (m_callback) m_callback->OnTaskFinished(this);
    return S_OK;
}

// ============================================================================
// FullTextRenderTask
// ============================================================================
FullTextRenderTask::FullTextRenderTask() = default;
FullTextRenderTask::~FullTextRenderTask() = default;

HRESULT FullTextRenderTask::Execute()
{
    m_state = TaskState::Running;

    IDWriteFactory* dwFactory = GetDWriteFactory();
    ID2D1Factory* d2dFactory = GetD2DFactory();
    if (!dwFactory || !d2dFactory) { m_state = TaskState::Failed; return E_FAIL; }

    CComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = dwFactory->CreateTextFormat(m_fontFamily.c_str(), nullptr,
        m_bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        m_italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, m_fontSize, L"en-us", &textFormat);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    switch (m_alignment)
    {
    case 0: textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); break;
    case 1: textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); break;
    case 2: textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING); break;
    }
    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    CComPtr<IDWriteTextLayout> textLayout;
    hr = dwFactory->CreateTextLayout(m_text.c_str(), static_cast<UINT32>(m_text.length()),
        textFormat, static_cast<float>(m_width), static_cast<float>(m_height), &textLayout);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    DWRITE_TEXT_METRICS metrics;
    textLayout->GetMetrics(&metrics);

    if (m_width == 0) m_width = static_cast<UINT>(ceilf(metrics.width) + 4);
    if (m_height == 0) m_height = static_cast<UINT>(ceilf(metrics.height) + 4);

    CComPtr<IWICImagingFactory> wicFactory = GetWICFactory();
    if (!wicFactory) { m_state = TaskState::Failed; return E_FAIL; }

    CComPtr<IWICBitmap> wicBitmap;
    hr = wicFactory->CreateBitmap(m_width, m_height, GUID_WICPixelFormat32bppBGRA,
        WICBitmapCacheOnDemand, &wicBitmap);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    CComPtr<ID2D1RenderTarget> rt;
    hr = d2dFactory->CreateWicBitmapRenderTarget(wicBitmap, rtProps, &rt);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    rt->BeginDraw();
    rt->Clear(D2D1::ColorF(0, 0, 0, 0));

    D2D1_COLOR_F fgColor = D2D1::ColorF(m_color.x, m_color.y, m_color.z, m_color.w);
    CComPtr<ID2D1SolidColorBrush> brush;
    rt->CreateSolidColorBrush(fgColor, &brush);

    D2D1_RECT_F layoutRect = D2D1::RectF(0, 0, (float)m_width, (float)m_height);
    rt->DrawTextLayout(D2D1::Point2F(0, 0), textLayout, brush);

    hr = rt->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) { m_state = TaskState::Failed; return hr; }

    CComPtr<IWICBitmapLock> lock;
    WICRect wicRect = { 0, 0, static_cast<INT>(m_width), static_cast<INT>(m_height) };
    hr = wicBitmap->Lock(&wicRect, WICBitmapLockRead, &lock);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    BYTE* data = nullptr;
    UINT bufferSize = 0;
    UINT stride = 0;
    lock->GetDataPointer(&bufferSize, &data);
    lock->GetStride(&stride);

    if (!m_device) { m_state = TaskState::Failed; return E_POINTER; }

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = m_width;
    td.Height = m_height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    std::vector<BYTE> pixelData(m_width * m_height * 4);
    hr = wicBitmap->CopyPixels(nullptr, m_width * 4, m_width * m_height * 4, pixelData.data());
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixelData.data();
    initData.SysMemPitch = m_width * 4;

    m_resultTexture.Release();
    m_resultSRV.Release();

    hr = m_device->CreateTexture2D(&td, &initData, &m_resultTexture);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    hr = m_device->CreateShaderResourceView(m_resultTexture, nullptr, &m_resultSRV);
    if (FAILED(hr)) { m_state = TaskState::Failed; return hr; }

    m_state = TaskState::Completed;
    if (m_callback) m_callback->OnTaskFinished(this);
    return S_OK;
}

} // namespace HMREngine
