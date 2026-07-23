#include "pch.h"
// TextResourceDX.cpp - D3D11 text rendering implementation

#include "TextResourceDX.h"
#include <wincodec.h>
#include <d2d1.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace HMREngine
{
namespace DX
{

// ============================================================================
// TextResourceBaseDX
// ============================================================================
TextResourceBaseDX::TextResourceBaseDX() = default;
TextResourceBaseDX::~TextResourceBaseDX() { Release(); }

HRESULT TextResourceBaseDX::Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
    m_device = dev;
    m_context = ctx;

    HRESULT hr = CreateD2DFactory();
    if (FAILED(hr)) return hr;

    hr = CreateDWriteFactory();
    if (FAILED(hr)) return hr;

    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory), (void**)&m_wicFactory);

    return hr;
}

void TextResourceBaseDX::Release()
{
    m_bitmap.Release();
    m_renderTarget.Release();
    m_wicBitmap.Release();
    m_wicFactory.Release();
    m_dwriteFactory.Release();
    m_d2dFactory.Release();
    m_bitmapWidth = 0;
    m_bitmapHeight = 0;
}

HRESULT TextResourceBaseDX::CreateD2DFactory()
{
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), (void**)&m_d2dFactory);
}

HRESULT TextResourceBaseDX::CreateDWriteFactory()
{
    return DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_dwriteFactory));
}

HRESULT TextResourceBaseDX::CreateWICBitmap(UINT width, UINT height)
{
    if (!m_wicFactory) return E_FAIL;

    m_bitmapWidth = width;
    m_bitmapHeight = height;

    m_wicBitmap.Release();
    HRESULT hr = m_wicFactory->CreateBitmap(width, height,
        GUID_WICPixelFormat32bppBGRA, WICBitmapCacheOnDemand, &m_wicBitmap);
    if (FAILED(hr)) return hr;

    if (!m_d2dFactory) return E_FAIL;

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    m_renderTarget.Release();
    hr = m_d2dFactory->CreateWicBitmapRenderTarget(m_wicBitmap, &rtProps, &m_renderTarget);
    return hr;
}

HRESULT TextResourceBaseDX::BeginTextLayout(UINT width, UINT height)
{
    return CreateWICBitmap(width, height);
}

HRESULT TextResourceBaseDX::EndTextLayout()
{
    if (m_renderTarget)
        m_renderTarget->Flush();
    return S_OK;
}

// ============================================================================
// TextResourceDX
// ============================================================================
TextResourceDX::TextResourceDX() = default;
TextResourceDX::~TextResourceDX() { Release(); }

HRESULT TextResourceDX::RenderText(const std::wstring& text,
    const std::wstring& fontFamily, float fontSize,
    const Rgba& color, bool bold, bool italic)
{
    if (!m_device || text.empty()) return E_INVALIDARG;

    IDWriteFactory* dwFactory = GetDWriteFactory();
    if (!dwFactory) return E_FAIL;

    CComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = dwFactory->CreateTextFormat(
        fontFamily.c_str(), nullptr,
        bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize, L"en-us", &textFormat);
    if (FAILED(hr)) return hr;

    textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    CComPtr<IDWriteRenderingParams> renderParams;
    dwFactory->CreateRenderingParams(&renderParams);

    CComPtr<IDWriteTextLayout> textLayout;
    hr = dwFactory->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.length()),
        textFormat, 4096.0f, 4096.0f, &textLayout);
    if (FAILED(hr)) return hr;

    DWRITE_TEXT_METRICS metrics;
    hr = textLayout->GetMetrics(&metrics);
    if (FAILED(hr)) return hr;

    UINT texWidth = static_cast<UINT>(ceilf(metrics.width) + 4);
    UINT texHeight = static_cast<UINT>(ceilf(metrics.height) + 4);
    if (texWidth < 4) texWidth = 4;
    if (texHeight < 4) texHeight = 4;

    hr = BeginTextLayout(texWidth, texHeight);
    if (FAILED(hr)) return hr;

    CComPtr<ID2D1RenderTarget> rt = GetRT();
    if (!rt) return E_FAIL;

    rt->BeginDraw();

    D2D1_COLOR_F d2dColor = D2D1::ColorF(color.x, color.y, color.z, color.w);
    CComPtr<ID2D1SolidColorBrush> brush;
    rt->CreateSolidColorBrush(d2dColor, &brush);

    D2D1_RECT_F layoutRect = D2D1::RectF(0.0f, 0.0f,
        static_cast<float>(texWidth), static_cast<float>(texHeight));

    rt->DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), textLayout, brush);

    hr = rt->EndDraw();
    if (FAILED(hr)) return hr;

    hr = EndTextLayout();
    if (FAILED(hr)) return hr;

    hr = CreateD3DTexture(texWidth, texHeight);
    if (FAILED(hr)) return hr;

    m_textWidth = texWidth;
    m_textHeight = texHeight;

    return UpdateTextureFromBitmap();
}

HRESULT TextResourceDX::CreateD3DTexture(UINT width, UINT height)
{
    m_texture.Release();
    m_srv.Release();

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_texture);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateShaderResourceView(m_texture, nullptr, &m_srv);
    return hr;
}

HRESULT TextResourceDX::UpdateTextureFromBitmap()
{
    if (!m_texture || !m_wicBitmap) return E_FAIL;

    IWICBitmapLock* lock = nullptr;
    WICRect rect = { 0, 0, static_cast<INT>(m_textWidth), static_cast<INT>(m_textHeight) };
    HRESULT hr = m_wicBitmap->Lock(&rect, WICBitmapLockRead, &lock);
    if (FAILED(hr)) return hr;

    BYTE* data = nullptr;
    UINT bufferSize = 0;
    UINT stride = 0;
    hr = lock->GetDataPointer(&bufferSize, &data);
    if (SUCCEEDED(hr)) hr = lock->GetStride(&stride);

    if (SUCCEEDED(hr))
    {
        m_context->UpdateSubresource(m_texture, 0, nullptr,
            data, stride, 0);
    }

    lock->Release();
    return hr;
}

HRESULT TextResourceDX::GetTexture(ID3D11Texture2D** ppTex) const
{
    if (!ppTex) return E_POINTER;
    if (!m_texture) return E_FAIL;
    *ppTex = m_texture;
    (*ppTex)->AddRef();
    return S_OK;
}

HRESULT TextResourceDX::GetSRV(ID3D11ShaderResourceView** ppSRV) const
{
    if (!ppSRV) return E_POINTER;
    if (!m_srv) return E_FAIL;
    *ppSRV = m_srv;
    (*ppSRV)->AddRef();
    return S_OK;
}

// ============================================================================
// ScrollingTextResourceDX
// ============================================================================
ScrollingTextResourceDX::ScrollingTextResourceDX() = default;
ScrollingTextResourceDX::~ScrollingTextResourceDX() { Release(); }

HRESULT ScrollingTextResourceDX::Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx,
    UINT viewportWidth, UINT viewportHeight)
{
    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;
    return TextResourceBaseDX::Initialize(dev, ctx);
}

void ScrollingTextResourceDX::Release()
{
    m_texture.Release();
    m_srv.Release();
    m_viewportWidth = 0;
    m_viewportHeight = 0;
    m_totalTextHeight = 0.0f;
    TextResourceBaseDX::Release();
}

HRESULT ScrollingTextResourceDX::CreateScrollingResources(UINT width, UINT height)
{
    m_texture.Release();
    m_srv.Release();

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_texture);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateShaderResourceView(m_texture, nullptr, &m_srv);
    return hr;
}

HRESULT ScrollingTextResourceDX::RenderScrollingText(
    const std::vector<std::wstring>& lines, float scrollOffset,
    const std::wstring& fontFamily, float fontSize, const Rgba& color)
{
    if (!m_device || !m_dwriteFactory || lines.empty()) return E_INVALIDARG;

    IDWriteFactory* dwFactory = GetDWriteFactory();

    CComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = dwFactory->CreateTextFormat(fontFamily.c_str(), nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &textFormat);
    if (FAILED(hr)) return hr;

    textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    float totalHeight = 0.0f;
    float maxWidth = 0.0f;

    std::vector<CComPtr<IDWriteTextLayout>> layouts(lines.size());
    for (size_t i = 0; i < lines.size(); ++i)
    {
        hr = dwFactory->CreateTextLayout(lines[i].c_str(),
            static_cast<UINT32>(lines[i].length()),
            textFormat, static_cast<float>(m_viewportWidth), 1024.0f, &layouts[i]);
        if (FAILED(hr)) return hr;

        DWRITE_TEXT_METRICS metrics;
        layouts[i]->GetMetrics(&metrics);
        totalHeight += metrics.height + fontSize * 0.5f;
        if (metrics.width > maxWidth) maxWidth = metrics.width;
    }

    m_totalTextHeight = totalHeight;

    UINT texWidth = m_viewportWidth;
    UINT texHeight = m_viewportHeight;

    hr = CreateScrollingResources(texWidth, texHeight);
    if (FAILED(hr)) return hr;

    hr = BeginTextLayout(texWidth, texHeight);
    if (FAILED(hr)) return hr;

    CComPtr<ID2D1RenderTarget> rt = GetRT();
    if (!rt) return E_FAIL;

    rt->BeginDraw();

    D2D1_COLOR_F d2dColor = D2D1::ColorF(color.x, color.y, color.z, color.w);
    CComPtr<ID2D1SolidColorBrush> brush;
    rt->CreateSolidColorBrush(d2dColor, &brush);

    float y = scrollOffset;
    for (size_t i = 0; i < layouts.size(); ++i)
    {
        if (!layouts[i]) continue;

        DWRITE_TEXT_METRICS metrics;
        layouts[i]->GetMetrics(&metrics);

        if (y + metrics.height > 0 && y < static_cast<float>(texHeight))
        {
            D2D1_RECT_F rect = D2D1::RectF(0.0f, y, static_cast<float>(texWidth), y + 2048.0f);
            rt->DrawTextLayout(D2D1::Point2F(0.0f, y), layouts[i], brush);
        }

        y += metrics.height + fontSize * 0.5f;
    }

    rt->EndDraw();
    hr = EndTextLayout();
    if (FAILED(hr)) return hr;

    if (m_texture && m_wicBitmap)
    {
        CComPtr<IWICBitmapLock> lock;
        WICRect rect = { 0, 0, static_cast<INT>(m_bitmapWidth), static_cast<INT>(m_bitmapHeight) };
        if (SUCCEEDED(m_wicBitmap->Lock(&rect, WICBitmapLockRead, &lock)))
        {
            BYTE* pixData = nullptr;
            UINT bufSize = 0;
            UINT stride = 0;
            if (SUCCEEDED(lock->GetDataPointer(&bufSize, &pixData)) && SUCCEEDED(lock->GetStride(&stride)))
            {
                m_context->UpdateSubresource(m_texture, 0, nullptr, pixData, stride, 0);
            }
        }
    }

    return S_OK;
}

HRESULT ScrollingTextResourceDX::GetTexture(ID3D11Texture2D** ppTex) const
{
    if (!ppTex) return E_POINTER;
    if (!m_texture) return E_FAIL;
    *ppTex = m_texture;
    (*ppTex)->AddRef();
    return S_OK;
}

HRESULT ScrollingTextResourceDX::GetSRV(ID3D11ShaderResourceView** ppSRV) const
{
    if (!ppSRV) return E_POINTER;
    if (!m_srv) return E_FAIL;
    *ppSRV = m_srv;
    (*ppSRV)->AddRef();
    return S_OK;
}

// ============================================================================
// PartialTextRendererDX
// ============================================================================
PartialTextRendererDX::PartialTextRendererDX() = default;
PartialTextRendererDX::~PartialTextRendererDX() { Release(); }

HRESULT PartialTextRendererDX::Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx,
    UINT canvasWidth, UINT canvasHeight)
{
    m_canvasWidth = canvasWidth;
    m_canvasHeight = canvasHeight;

    HRESULT hr = TextResourceBaseDX::Initialize(dev, ctx);
    if (FAILED(hr)) return hr;

    return CreateCanvasResources(canvasWidth, canvasHeight);
}

void PartialTextRendererDX::Release()
{
    m_texture.Release();
    m_srv.Release();
    m_canvasWidth = 0;
    m_canvasHeight = 0;
    TextResourceBaseDX::Release();
}

HRESULT PartialTextRendererDX::CreateCanvasResources(UINT width, UINT height)
{
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_texture);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateShaderResourceView(m_texture, nullptr, &m_srv);
    return hr;
}

HRESULT PartialTextRendererDX::RenderPartialText(const std::wstring& text,
    float x, float y, const std::wstring& fontFamily, float fontSize, const Rgba& color)
{
    if (!m_device || !m_dwriteFactory || text.empty()) return E_INVALIDARG;

    IDWriteFactory* dwFactory = GetDWriteFactory();

    CComPtr<IDWriteTextFormat> textFormat;
    HRESULT hr = dwFactory->CreateTextFormat(fontFamily.c_str(), nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &textFormat);
    if (FAILED(hr)) return hr;

    CComPtr<IDWriteTextLayout> textLayout;
    hr = dwFactory->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.length()),
        textFormat, 4096.0f, 1024.0f, &textLayout);
    if (FAILED(hr)) return hr;

    DWRITE_TEXT_METRICS metrics;
    textLayout->GetMetrics(&metrics);

    UINT regionWidth = static_cast<UINT>(ceilf(metrics.width) + 4);
    UINT regionHeight = static_cast<UINT>(ceilf(metrics.height) + 4);

    hr = BeginTextLayout(regionWidth, regionHeight);
    if (FAILED(hr)) return hr;

    CComPtr<ID2D1RenderTarget> rt = GetRT();
    if (!rt) return E_FAIL;

    rt->BeginDraw();

    D2D1_COLOR_F d2dColor = D2D1::ColorF(color.x, color.y, color.z, color.w);
    CComPtr<ID2D1SolidColorBrush> brush;
    rt->CreateSolidColorBrush(d2dColor, &brush);

    rt->DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), textLayout, brush);

    hr = rt->EndDraw();
    if (FAILED(hr)) return hr;

    hr = EndTextLayout();
    if (FAILED(hr)) return hr;

    if (m_texture && m_wicBitmap)
    {
        CComPtr<IWICBitmapLock> lock;
        WICRect rect = { static_cast<INT>(x), static_cast<INT>(y), static_cast<INT>(regionWidth), static_cast<INT>(regionHeight) };
        if (SUCCEEDED(m_wicBitmap->Lock(&rect, WICBitmapLockRead, &lock)))
        {
            BYTE* pixData = nullptr;
            UINT bufSize = 0;
            UINT stride = 0;
            if (SUCCEEDED(lock->GetDataPointer(&bufSize, &pixData)) && SUCCEEDED(lock->GetStride(&stride)))
            {
                D3D11_BOX box = { static_cast<UINT>(x), static_cast<UINT>(y), 0,
                                  static_cast<UINT>(x) + regionWidth, static_cast<UINT>(y) + regionHeight, 1 };
                m_context->UpdateSubresource(m_texture, 0, &box, pixData, stride, 0);
            }
        }
    }

    return S_OK;
}

HRESULT PartialTextRendererDX::Clear()
{
    if (!m_context || !m_texture) return S_OK;

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    CComPtr<ID3D11RenderTargetView> rtv;
    HRESULT hr = m_device->CreateRenderTargetView(m_texture, nullptr, &rtv);
    if (SUCCEEDED(hr))
    {
        m_context->ClearRenderTargetView(rtv, clearColor);
    }
    return hr;
}

HRESULT PartialTextRendererDX::GetTexture(ID3D11Texture2D** ppTex) const
{
    if (!ppTex) return E_POINTER;
    if (!m_texture) return E_FAIL;
    *ppTex = m_texture;
    (*ppTex)->AddRef();
    return S_OK;
}

HRESULT PartialTextRendererDX::GetSRV(ID3D11ShaderResourceView** ppSRV) const
{
    if (!ppSRV) return E_POINTER;
    if (!m_srv) return E_FAIL;
    *ppSRV = m_srv;
    (*ppSRV)->AddRef();
    return S_OK;
}

} // namespace DX
} // namespace HMREngine
