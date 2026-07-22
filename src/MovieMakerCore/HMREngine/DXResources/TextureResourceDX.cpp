#include "pch.h"
// TextureResourceDX.cpp - D3D11 texture resource implementation

#include "TextureResourceDX.h"
#include <wincodec.h>
#include <wrl/client.h>

#pragma comment(lib, "windowscodecs.lib")

using namespace Microsoft::WRL;

namespace HMREngine
{
namespace DX
{

// ============================================================================
// TextureResourceDX
// ============================================================================
TextureResourceDX::TextureResourceDX() = default;
TextureResourceDX::~TextureResourceDX() { Release(); }

HRESULT TextureResourceDX::LoadFromFile(const std::wstring& filePath)
{
    m_filePath = filePath;
    m_loaded = false;

    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    std::vector<BYTE> fileData(static_cast<size_t>(fileSize.QuadPart));
    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, fileData.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr);
    CloseHandle(hFile);
    if (!ok) return HRESULT_FROM_WIN32(GetLastError());

    HRESULT hr = CreateTextureFromWIC(fileData.data(), static_cast<UINT>(fileData.size()));
    if (SUCCEEDED(hr)) m_loaded = true;
    return hr;
}

HRESULT TextureResourceDX::LoadFromFile(const std::string& filePath)
{
    int len = MultiByteToWideChar(CP_ACP, 0, filePath.c_str(), -1, nullptr, 0);
    std::wstring wPath(len, 0);
    MultiByteToWideChar(CP_ACP, 0, filePath.c_str(), -1, &wPath[0], len);
    wPath.resize(len - 1);
    return LoadFromFile(wPath);
}

HRESULT TextureResourceDX::CreateFromData(const void* data, UINT width, UINT height,
    DXGI_FORMAT format, UINT rowPitch)
{
    if (!data || !m_device) return E_INVALIDARG;

    Release();

    m_width = width;
    m_height = height;
    m_format = format;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = format;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    UINT actualRowPitch = rowPitch ? rowPitch : width * 4;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;
    initData.SysMemPitch = actualRowPitch;

    HRESULT hr = m_device->CreateTexture2D(&td, &initData, &m_texture);
    if (FAILED(hr)) return hr;

    hr = CreateSRV();
    if (SUCCEEDED(hr)) m_loaded = true;
    return hr;
}

HRESULT TextureResourceDX::CreateEmpty(UINT width, UINT height, DXGI_FORMAT format)
{
    if (!m_device) return E_FAIL;

    Release();

    m_width = width;
    m_height = height;
    m_format = format;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = format;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_texture);
    if (FAILED(hr)) return hr;

    hr = CreateSRV();
    if (SUCCEEDED(hr)) m_loaded = true;
    return hr;
}

HRESULT TextureResourceDX::CreateFromDIB(const void* dibData, UINT dataSize)
{
    if (!dibData || dataSize < sizeof(BITMAPINFOHEADER)) return E_INVALIDARG;

    const BITMAPINFOHEADER* bih = static_cast<const BITMAPINFOHEADER*>(dibData);
    if (bih->biCompression != BI_RGB) return E_NOTIMPL;

    UINT width = bih->biWidth;
    UINT height = abs(bih->biHeight);
    bool topDown = bih->biHeight < 0;
    UINT bpp = bih->biBitCount;

    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    UINT pixelSize = 0;
    switch (bpp)
    {
    case 32: format = DXGI_FORMAT_B8G8R8A8_UNORM; pixelSize = 4; break;
    case 24: format = DXGI_FORMAT_B8G8R8X8_UNORM; pixelSize = 3; break;
    default: return E_NOTIMPL;
    }

    const BYTE* pixelData = static_cast<const BYTE*>(dibData) + bih->biSize;

    std::vector<BYTE> rgba(width * height * 4);
    UINT srcRowPitch = ((width * pixelSize + 3) & ~3);

    for (UINT y = 0; y < height; ++y)
    {
        UINT srcY = topDown ? y : (height - 1 - y);
        const BYTE* srcRow = pixelData + srcY * srcRowPitch;
        BYTE* dstRow = rgba.data() + y * width * 4;

        for (UINT x = 0; x < width; ++x)
        {
            const BYTE* srcPx = srcRow + x * pixelSize;
            dstRow[x * 4 + 0] = srcPx[0];
            dstRow[x * 4 + 1] = srcPx[1];
            dstRow[x * 4 + 2] = pixelSize >= 3 ? srcPx[2] : srcPx[0];
            dstRow[x * 4 + 3] = 0xFF;
        }
    }

    return CreateFromData(rgba.data(), width, height, DXGI_FORMAT_B8G8R8A8_UNORM, width * 4);
}

HRESULT TextureResourceDX::CreateTextureFromWIC(const void* data, UINT size)
{
    static HRESULT s_wicResult = []() -> HRESULT
    {
        return CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    }();

    CComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory), (void**)&wicFactory);
    if (FAILED(hr)) return hr;

    CComPtr<IStream> stream;
    hr = CreateStreamOnHGlobal(nullptr, TRUE, &stream);
    if (FAILED(hr)) return hr;

    hr = stream->Write(data, size, nullptr);
    if (FAILED(hr)) return hr;

    LARGE_INTEGER li = {};
    li.QuadPart = 0;
    hr = stream->Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) return hr;

    CComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) return hr;

    CComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return hr;

    UINT w = 0, h = 0;
    frame->GetSize(&w, &h);
    m_width = w;
    m_height = h;
    m_format = DXGI_FORMAT_B8G8R8A8_UNORM;

    CComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (SUCCEEDED(hr))
    {
        hr = converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    }

    if (FAILED(hr)) return hr;

    std::vector<BYTE> pixels(w * h * 4);
    hr = converter->CopyPixels(nullptr, w * 4, w * h * 4, pixels.data());
    if (FAILED(hr)) return hr;

    return CreateFromData(pixels.data(), w, h, DXGI_FORMAT_B8G8R8A8_UNORM, w * 4);
}

HRESULT TextureResourceDX::CreateSRV()
{
    if (!m_texture) return E_FAIL;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = m_format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;

    return m_device->CreateShaderResourceView(m_texture, &srvd, &m_srv);
}

void TextureResourceDX::Release()
{
    m_srv.Release();
    m_texture.Release();
    m_loaded = false;
    m_width = 0;
    m_height = 0;
    m_format = DXGI_FORMAT_UNKNOWN;
}

HRESULT TextureResourceDX::GetTexture(ID3D11Texture2D** ppTex) const
{
    if (!ppTex) return E_POINTER;
    if (!m_texture) return E_FAIL;
    *ppTex = m_texture;
    (*ppTex)->AddRef();
    return S_OK;
}

HRESULT TextureResourceDX::GetSRV(ID3D11ShaderResourceView** ppSRV) const
{
    if (!ppSRV) return E_POINTER;
    if (!m_srv) return E_FAIL;
    *ppSRV = m_srv;
    (*ppSRV)->AddRef();
    return S_OK;
}

// ============================================================================
// TextureResourceFromResourceDX
// ============================================================================
TextureResourceFromResourceDX::TextureResourceFromResourceDX() = default;
TextureResourceFromResourceDX::~TextureResourceFromResourceDX() = default;

HRESULT TextureResourceFromResourceDX::LoadFromResource(HMODULE hModule, UINT resourceId,
    const std::wstring& resourceType)
{
    std::vector<BYTE> data;
    HRESULT hr = ExtractResourceData(hModule, resourceId, resourceType, data);
    if (FAILED(hr)) return hr;

    hr = CreateTextureFromWIC(data.data(), static_cast<UINT>(data.size()));
    if (SUCCEEDED(hr))
    {
        m_loaded = true;
        m_resourceModule = hModule;
        m_resourceId = resourceId;
    }
    return hr;
}

HRESULT TextureResourceFromResourceDX::LoadFromResource(HMODULE hModule,
    const std::wstring& resourceName, const std::wstring& resourceType)
{
    HRSRC hRes = FindResourceW(hModule, resourceName.c_str(), resourceType.c_str());
    if (!hRes) return HRESULT_FROM_WIN32(GetLastError());

    HGLOBAL hGlobal = LoadResource(hModule, hRes);
    if (!hGlobal) return HRESULT_FROM_WIN32(GetLastError());

    DWORD size = SizeofResource(hModule, hRes);
    const void* data = LockResource(hGlobal);
    if (!data) return E_FAIL;

    HRESULT hr = CreateTextureFromWIC(data, size);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);

    if (SUCCEEDED(hr)) m_loaded = true;
    return hr;
}

HRESULT TextureResourceFromResourceDX::ExtractResourceData(HMODULE hModule, UINT resourceId,
    const std::wstring& resourceType, std::vector<BYTE>& outData)
{
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(resourceId), resourceType.c_str());
    if (!hRes) return HRESULT_FROM_WIN32(GetLastError());

    HGLOBAL hGlobal = LoadResource(hModule, hRes);
    if (!hGlobal) return HRESULT_FROM_WIN32(GetLastError());

    DWORD size = SizeofResource(hModule, hRes);
    const void* data = LockResource(hGlobal);
    if (!data) { FreeResource(hGlobal); return E_FAIL; }

    outData.assign(static_cast<const BYTE*>(data), static_cast<const BYTE*>(data) + size);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);

    return S_OK;
}

// ============================================================================
// MotionTextureResourceDX
// ============================================================================
MotionTextureResourceDX::MotionTextureResourceDX() = default;
MotionTextureResourceDX::~MotionTextureResourceDX() { Release(); }

HRESULT MotionTextureResourceDX::LoadFromFiles(const std::vector<std::wstring>& framePaths)
{
    if (framePaths.empty()) return E_INVALIDARG;
    if (!m_device) return E_FAIL;

    Release();

    m_frameCount = static_cast<UINT>(framePaths.size());
    m_isStrip = false;
    m_frames.resize(m_frameCount);
    m_srvs.resize(m_frameCount);

    for (UINT i = 0; i < m_frameCount; ++i)
    {
        TextureResourceDX tex;
        tex.InitializeDevice(m_device, m_context);
        HRESULT hr = tex.LoadFromFile(framePaths[i]);
        if (FAILED(hr)) return hr;

        tex.GetTexture(&m_frames[i]);
        tex.GetSRV(&m_srvs[i]);

        if (m_frameWidth == 0)
        {
            m_frameWidth = tex.GetWidth();
            m_frameHeight = tex.GetHeight();
        }
    }

    return S_OK;
}

HRESULT MotionTextureResourceDX::LoadFromStrip(const std::wstring& stripPath,
    UINT frameCount, UINT frameWidth, UINT frameHeight)
{
    if (!m_device || frameCount == 0) return E_INVALIDARG;

    Release();

    TextureResourceDX stripTex;
    stripTex.InitializeDevice(m_device, m_context);
    HRESULT hr = stripTex.LoadFromFile(stripPath);
    if (FAILED(hr)) return hr;

    hr = stripTex.GetTexture(&m_stripTexture);
    if (FAILED(hr)) return hr;

    m_frameCount = frameCount;
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_isStrip = true;

    D3D11_TEXTURE2D_DESC srcDesc;
    m_stripTexture->GetDesc(&srcDesc);

    D3D11_TEXTURE2D_DESC frameDesc = {};
    frameDesc.Width = frameWidth;
    frameDesc.Height = frameHeight;
    frameDesc.MipLevels = 1;
    frameDesc.ArraySize = 1;
    frameDesc.Format = srcDesc.Format;
    frameDesc.SampleDesc.Count = 1;
    frameDesc.Usage = D3D11_USAGE_DEFAULT;
    frameDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    m_frames.resize(frameCount);
    m_srvs.resize(frameCount);

    for (UINT i = 0; i < frameCount; ++i)
    {
        hr = m_device->CreateTexture2D(&frameDesc, nullptr, &m_frames[i]);
        if (FAILED(hr)) return hr;

        hr = m_device->CreateShaderResourceView(m_frames[i], nullptr, &m_srvs[i]);
        if (FAILED(hr)) return hr;
    }

    return S_OK;
}

HRESULT MotionTextureResourceDX::CreateFrameFromStripData(const BYTE* stripData,
    UINT stripSize, UINT frameIndex, ID3D11Texture2D** ppTex)
{
    if (!stripData || !ppTex || !m_device) return E_POINTER;
    if (frameIndex >= m_frameCount) return E_INVALIDARG;
    if (m_frameWidth == 0 || m_frameHeight == 0) return E_FAIL;

    D3D11_TEXTURE2D_DESC srcDesc = {};
    if (m_stripTexture)
        m_stripTexture->GetDesc(&srcDesc);
    else
        return E_FAIL;

    D3D11_TEXTURE2D_DESC frameDesc = {};
    frameDesc.Width = m_frameWidth;
    frameDesc.Height = m_frameHeight;
    frameDesc.MipLevels = 1;
    frameDesc.ArraySize = 1;
    frameDesc.Format = srcDesc.Format;
    frameDesc.SampleDesc.Count = 1;
    frameDesc.Usage = D3D11_USAGE_DEFAULT;
    frameDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    std::vector<BYTE> framePixels(m_frameWidth * m_frameHeight * 4);

    UINT bytesPerPixel = 4;
    if (srcDesc.Format == DXGI_FORMAT_B5G6R5_UNORM || srcDesc.Format == DXGI_FORMAT_B5G5R5A1_UNORM || srcDesc.Format == DXGI_FORMAT_B4G4R4A4_UNORM)
        bytesPerPixel = 2;

    UINT stripRowPitch = srcDesc.Width * bytesPerPixel;
    UINT frameRowPitch = m_frameWidth * bytesPerPixel;

    for (UINT y = 0; y < m_frameHeight; ++y)
    {
        UINT srcOffset = (frameIndex * m_frameWidth + y * stripRowPitch);
        if (srcOffset + frameRowPitch > stripSize) break;

        memcpy(framePixels.data() + y * frameRowPitch, stripData + srcOffset, frameRowPitch);
    }

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = framePixels.data();
    initData.SysMemPitch = frameRowPitch;

    HRESULT hr = m_device->CreateTexture2D(&frameDesc, &initData, ppTex);
    return hr;
}

HRESULT MotionTextureResourceDX::GetTextureForFrame(UINT frameIndex, ID3D11ShaderResourceView** ppSRV)
{
    if (!ppSRV) return E_POINTER;
    if (frameIndex >= m_frameCount) return E_INVALIDARG;
    if (!m_srvs[frameIndex]) return E_FAIL;

    *ppSRV = m_srvs[frameIndex];
    (*ppSRV)->AddRef();
    return S_OK;
}

HRESULT MotionTextureResourceDX::GetTextureAtTime(double time, UINT fps, ID3D11ShaderResourceView** ppSRV)
{
    if (m_frameCount == 0) return E_FAIL;

    UINT frame = static_cast<UINT>(time * fps) % m_frameCount;

    return GetTextureForFrame(frame, ppSRV);
}

void MotionTextureResourceDX::Release()
{
    m_srvs.clear();
    m_frames.clear();
    m_stripTexture.Release();
    m_frameCount = 0;
    m_frameWidth = 0;
    m_frameHeight = 0;
    m_isStrip = false;
}

} // namespace DX
} // namespace HMREngine
