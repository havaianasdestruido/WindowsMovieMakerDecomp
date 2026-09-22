#include "pch.h"
#include "d3dx11compat.h"
#include <fstream>
#include <vector>

extern "C" {

// ============================================================================
// D3DX11CompileEffectFromFileA
// Reads an HLSL effect file from disk and compiles it via D3DCompile + D3DX11CreateEffectFromMemory.
// ============================================================================
HRESULT WINAPI D3DX11CompileEffectFromFileA(
    LPCSTR pFileName,
    const D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    DWORD Flags,
    DWORD FXFlags,
    ID3D11Device* pDevice,
    ID3DX11Effect** ppEffect,
    ID3DBlob** ppErrorMsgs,
    void* pPool)
{
    if (!pFileName || !pDevice || !ppEffect) return E_INVALIDARG;

    HANDLE hFile = CreateFileA(pFileName, GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    if (fileSize.QuadPart == 0 || fileSize.QuadPart > 0x7FFFFFFF)
    {
        CloseHandle(hFile);
        return E_FAIL;
    }

    std::vector<BYTE> fileData(static_cast<size_t>(fileSize.QuadPart));
    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, fileData.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr);
    CloseHandle(hFile);
    if (!ok) return HRESULT_FROM_WIN32(GetLastError());

    *ppEffect = nullptr;

    CComPtr<ID3DBlob> shaderBlob;
    CComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(
        fileData.data(), fileData.size(),
        pFileName, pDefines, pInclude,
        "", "fx_5_0", Flags, 0, &shaderBlob, &errorBlob);

    if (ppErrorMsgs && errorBlob)
    {
        *ppErrorMsgs = errorBlob;
        (*ppErrorMsgs)->AddRef();
    }

    if (FAILED(hr)) return hr;

    hr = D3DX11CreateEffectFromMemory(
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        FXFlags, pDevice, ppEffect, nullptr);

    return hr;
}

// ============================================================================
// D3DX11CompileEffectFromFileW - Wide-char wrapper
// ============================================================================
HRESULT WINAPI D3DX11CompileEffectFromFileW(
    LPCWSTR pFileName,
    const D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    DWORD Flags,
    DWORD FXFlags,
    ID3D11Device* pDevice,
    ID3DX11Effect** ppEffect,
    ID3DBlob** ppErrorMsgs,
    void* pPool)
{
    if (!pFileName || !pDevice || !ppEffect) return E_INVALIDARG;

    HANDLE hFile = CreateFileW(pFileName, GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    if (fileSize.QuadPart == 0 || fileSize.QuadPart > 0x7FFFFFFF)
    {
        CloseHandle(hFile);
        return E_FAIL;
    }

    std::vector<BYTE> fileData(static_cast<size_t>(fileSize.QuadPart));
    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, fileData.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr);
    CloseHandle(hFile);
    if (!ok) return HRESULT_FROM_WIN32(GetLastError());

    *ppEffect = nullptr;

    CComPtr<ID3DBlob> shaderBlob;
    CComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(
        fileData.data(), fileData.size(),
        "effect", pDefines, pInclude,
        "", "fx_5_0", Flags, 0, &shaderBlob, &errorBlob);

    if (ppErrorMsgs && errorBlob)
    {
        *ppErrorMsgs = errorBlob;
        (*ppErrorMsgs)->AddRef();
    }

    if (FAILED(hr)) return hr;

    hr = D3DX11CreateEffectFromMemory(
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        FXFlags, pDevice, ppEffect, nullptr);

    return hr;
}

// ============================================================================
// D3DX11CompileEffectFromMemory - Compile from memory buffer
// ============================================================================
HRESULT WINAPI D3DX11CompileEffectFromMemory(
    const void* pData,
    UINT DataLen,
    LPCSTR pName,
    const D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    DWORD Flags,
    DWORD FXFlags,
    ID3D11Device* pDevice,
    ID3DX11Effect** ppEffect,
    ID3DBlob** ppErrorMsgs,
    void* pPool)
{
    if (!pData || DataLen == 0 || !pDevice || !ppEffect) return E_INVALIDARG;

    *ppEffect = nullptr;

    CComPtr<ID3DBlob> shaderBlob;
    CComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(
        pData, DataLen,
        pName ? pName : "effect_memory", pDefines, pInclude,
        "", "fx_5_0", Flags, 0, &shaderBlob, &errorBlob);

    if (ppErrorMsgs && errorBlob)
    {
        *ppErrorMsgs = errorBlob;
        (*ppErrorMsgs)->AddRef();
    }

    if (FAILED(hr)) return hr;

    hr = D3DX11CreateEffectFromMemory(
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        FXFlags, pDevice, ppEffect, nullptr);

    return hr;
}

// ============================================================================
// TODO(reconstruction): Replace this compatibility fallback if an effect-framework replacement can
// match the reference interface and behavior. D3DX11 no longer supplies the original framework.
// ============================================================================
HRESULT WINAPI D3DX11CreateEffectFromMemory(
    LPCVOID pData,
    SIZE_T DataLength,
    UINT Flags,
    ID3D11Device* pDevice,
    ID3DX11Effect** ppEffect,
    void* pEffectPool)
{
    UNREFERENCED_PARAMETER(pEffectPool);
    if (ppEffect) *ppEffect = nullptr;
    return E_NOTIMPL;
}

// ============================================================================
// D3DX11CreateShaderResourceViewFromFileA
// Loads an image file via WIC and creates a D3D11 SRV.
// ============================================================================
HRESULT WINAPI D3DX11CreateShaderResourceViewFromFileA(
    ID3D11Device* pDevice,
    LPCSTR pSrcFile,
    const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc,
    ID3D11DeviceContext* pContext,
    ID3D11ShaderResourceView** ppShaderResourceView,
    void* pPool)
{
    if (!pDevice || !pSrcFile || !ppShaderResourceView) return E_INVALIDARG;

    *ppShaderResourceView = nullptr;

    HANDLE hFile = CreateFileA(pSrcFile, GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    std::vector<BYTE> fileData(static_cast<size_t>(fileSize.QuadPart));
    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, fileData.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr);
    CloseHandle(hFile);
    if (!ok) return HRESULT_FROM_WIN32(GetLastError());

    static HRESULT s_wicResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    CComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory), (void**)&wicFactory);
    if (FAILED(hr)) return hr;

    CComPtr<IStream> stream;
    hr = CreateStreamOnHGlobal(nullptr, TRUE, &stream);
    if (FAILED(hr)) return hr;

    stream->Write(fileData.data(), static_cast<ULONG>(fileData.size()), nullptr);
    LARGE_INTEGER li = {};
    li.QuadPart = 0;
    stream->Seek(li, STREAM_SEEK_SET, nullptr);

    CComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) return hr;

    CComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return hr;

    UINT w = 0, h = 0;
    frame->GetSize(&w, &h);

    CComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return hr;

    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return hr;

    std::vector<BYTE> pixels(w * h * 4);
    hr = converter->CopyPixels(nullptr, w * 4, w * h * 4, pixels.data());
    if (FAILED(hr)) return hr;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = w;
    td.Height = h;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = w * 4;

    CComPtr<ID3D11Texture2D> texture;
    hr = pDevice->CreateTexture2D(&td, &initData, &texture);
    if (FAILED(hr)) return hr;

    if (pDesc)
    {
        hr = pDevice->CreateShaderResourceView(texture, pDesc, ppShaderResourceView);
    }
    else
    {
        hr = pDevice->CreateShaderResourceView(texture, nullptr, ppShaderResourceView);
    }

    return hr;
}

} // extern "C"
