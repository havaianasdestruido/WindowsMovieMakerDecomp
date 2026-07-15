#include "pch.h"
#include "d3dx11compat.h"

extern "C" {

HRESULT WINAPI D3DX11CompileEffectFromFileA(
    LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*,
    DWORD, DWORD, ID3D11Device*, ID3DX11Effect**, ID3DBlob**, void*)
{
    return E_NOTIMPL;
}

HRESULT WINAPI D3DX11CompileEffectFromFileW(
    LPCWSTR, const D3D_SHADER_MACRO*, ID3DInclude*,
    DWORD, DWORD, ID3D11Device*, ID3DX11Effect**, ID3DBlob**, void*)
{
    return E_NOTIMPL;
}

HRESULT WINAPI D3DX11CompileEffectFromMemory(
    const void*, UINT, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*,
    DWORD, DWORD, ID3D11Device*, ID3DX11Effect**, ID3DBlob**, void*)
{
    return E_NOTIMPL;
}

HRESULT WINAPI D3DX11CreateShaderResourceViewFromFileA(
    ID3D11Device*, LPCSTR, const D3D11_SHADER_RESOURCE_VIEW_DESC*,
    ID3D11DeviceContext*, ID3D11ShaderResourceView**, void*)
{
    return E_NOTIMPL;
}

} // extern "C"
