#pragma once
// d3dx11compat.h - Minimal D3DX11 Effect Framework compatibility declarations
// Provides the ID3DX11Effect* interface types needed by this project.
// These were originally in <d3dx11.h> from the legacy DirectX SDK.
// For runtime, link against the Effects11 (FX11) library.

#include <d3d11.h>
#include <d3dcompiler.h>

#ifndef D3DX11_EFFECT_DESC_DEFINED
#define D3DX11_EFFECT_DESC_DEFINED
typedef struct _D3DX11_EFFECT_DESC {
    UINT Techniques;
    UINT GlobalVariables;
    UINT ConstantBuffers;
    UINT Interfaces;
    UINT SharedInterfaces;
    UINT SharedTypes;
} D3DX11_EFFECT_DESC;
#endif

#ifndef D3DX11_TECHNIQUE_DESC_DEFINED
#define D3DX11_TECHNIQUE_DESC_DEFINED
typedef struct _D3DX11_TECHNIQUE_DESC {
    LPCSTR Name;
    UINT Passes;
    UINT Annotations;
} D3DX11_TECHNIQUE_DESC;
#endif

#ifndef D3DX11_PASS_DESC_DEFINED
#define D3DX11_PASS_DESC_DEFINED
typedef struct _D3DX11_PASS_DESC {
    LPCSTR Name;
    UINT Annotations;
} D3DX11_PASS_DESC;
#endif

struct ID3DX11EffectTechnique;
struct ID3DX11EffectPass;
struct ID3DX11EffectVariable;
struct ID3DX11EffectScalarVariable;
struct ID3DX11EffectVectorVariable;
struct ID3DX11EffectMatrixVariable;
struct ID3DX11EffectShaderResourceVariable;
struct ID3DX11EffectPooling;

MIDL_INTERFACE("c52072e9-445c-42d2-85d4-417e97b09059")
ID3DX11EffectPooling : public IUnknown
{
    virtual UINT STDMETHODCALLTYPE GetPoolSize() = 0;
    virtual HRESULT STDMETHODCALLTYPE SetTexture(UINT Slot, ID3D11ShaderResourceView* pResource) = 0;
};

MIDL_INTERFACE("e48f7b33-4ab8-4f0a-a26a-42c534398740")
ID3DX11EffectVariable : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetDesc(void* pDesc) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetAnnotationByIndex(UINT Index) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetAnnotationByName(LPCSTR Name) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetMemberByIndex(UINT Index) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetMemberByName(LPCSTR Name) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetMemberBySemantic(LPCSTR Semantic) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetElement(UINT Index) = 0;
    virtual ID3DX11EffectScalarVariable* STDMETHODCALLTYPE AsScalar() = 0;
    virtual ID3DX11EffectVectorVariable* STDMETHODCALLTYPE AsVector() = 0;
    virtual ID3DX11EffectMatrixVariable* STDMETHODCALLTYPE AsMatrix() = 0;
    virtual ID3DX11EffectShaderResourceVariable* STDMETHODCALLTYPE AsShaderResource() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRawValue(void* pData, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetRawValue(const void* pData, UINT Offset, UINT Count) = 0;
};

MIDL_INTERFACE("e8e4df1c-3b01-464f-883a-2b26bae70109")
ID3DX11EffectScalarVariable : public ID3DX11EffectVariable
{
    virtual HRESULT STDMETHODCALLTYPE GetFloat(float* pFloat) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFloatArray(float* pFloats, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFloat(float Float) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFloatArray(const float* pFloats, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetInt(int* pInt) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetIntArray(int* pInts, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetInt(int Int) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetIntArray(const int* pInts, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBool(BOOL* pBool) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBoolArray(BOOL* pBools, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBool(BOOL Bool) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBoolArray(const BOOL* pBools, UINT Offset, UINT Count) = 0;
};

MIDL_INTERFACE("c0e74b22-a6fa-40eb-8470-caf2c1120dba")
ID3DX11EffectVectorVariable : public ID3DX11EffectVariable
{
    virtual HRESULT STDMETHODCALLTYPE GetBoolVector(BOOL* pBool) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBoolVectorArray(BOOL* pBools, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetIntVector(int* pInt) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetIntArrayVector(int* pInts, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFloatVector(float* pFloat) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFloatVectorArray(float* pFloats, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBoolVector(const BOOL* pBool) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBoolVectorArray(const BOOL* pBools, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetIntVector(const int* pInt) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetIntVectorArray(const int* pInts, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFloatVector(const float* pFloat) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFloatVectorArray(const float* pFloats, UINT Offset, UINT Count) = 0;
};

MIDL_INTERFACE("e13930ff-2d1d-47f7-8a29-4bde1443596e")
ID3DX11EffectMatrixVariable : public ID3DX11EffectVariable
{
    virtual HRESULT STDMETHODCALLTYPE GetMatrix(float* pMatrix) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMatrixArray(float* pMatrix, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMatrixTranspose(float* pMatrix) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMatrixTransposeArray(float* pMatrix, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMatrix(const float* pMatrix) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMatrixArray(const float* pMatrix, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMatrixTranspose(const float* pMatrix) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMatrixTransposeArray(const float* pMatrix, UINT Offset, UINT Count) = 0;
};

MIDL_INTERFACE("c3dabd58-3b58-4c56-a017-28af540ff75d")
ID3DX11EffectShaderResourceVariable : public ID3DX11EffectVariable
{
    virtual HRESULT STDMETHODCALLTYPE GetResource(ID3D11ShaderResourceView** ppResource) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetResourceArray(ID3D11ShaderResourceView** ppResources, UINT Offset, UINT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetResource(ID3D11ShaderResourceView* pResource) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetResourceArray(ID3D11ShaderResourceView** ppResources, UINT Offset, UINT Count) = 0;
};

MIDL_INTERFACE("db6dec14-2b65-4d29-a663-3079f3e9d4cb")
ID3DX11EffectPass : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DX11_PASS_DESC* pDesc) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetAnnotationByIndex(UINT Index) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetAnnotationByName(LPCSTR Name) = 0;
    virtual HRESULT STDMETHODCALLTYPE Apply(UINT Flags, ID3D11DeviceContext* pContext) = 0;
    virtual HRESULT STDMETHODCALLTYPE ComputeStateBlock(ID3D11DeviceContext* pContext) = 0;
};

MIDL_INTERFACE("b2090694-6b6f-48f0-a317-8bfa5b32c1cb")
ID3DX11EffectTechnique : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DX11_TECHNIQUE_DESC* pDesc) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetAnnotationByIndex(UINT Index) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetAnnotationByName(LPCSTR Name) = 0;
    virtual ID3DX11EffectPass* STDMETHODCALLTYPE GetPassByIndex(UINT Index) = 0;
    virtual ID3DX11EffectPass* STDMETHODCALLTYPE GetPassByName(LPCSTR Name) = 0;
    virtual ID3DX11EffectPass* STDMETHODCALLTYPE GetPassBySemantic(LPCSTR Semantic) = 0;
};

MIDL_INTERFACE("42722be4-ea74-40e0-af7e-78f10b735899")
ID3DX11Effect : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DX11_EFFECT_DESC* pDesc) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetVariableByIndex(UINT Index) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetVariableByName(LPCSTR Name) = 0;
    virtual ID3DX11EffectVariable* STDMETHODCALLTYPE GetVariableBySemantic(LPCSTR Semantic) = 0;
    virtual ID3DX11EffectTechnique* STDMETHODCALLTYPE GetTechniqueByIndex(UINT Index) = 0;
    virtual ID3DX11EffectTechnique* STDMETHODCALLTYPE GetTechniqueByName(LPCSTR Name) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConstantBufferByIndex(UINT Index) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConstantBufferByName(LPCSTR Name) = 0;
    virtual HRESULT STDMETHODCALLTYPE CloneResource(ID3DX11Effect** ppEffect) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetOptimizedLevel() = 0;
    virtual BOOL STDMETHODCALLTYPE IsValid() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) = 0;
};

#ifndef D3DX11_IMAGE_INFO_DEFINED
#define D3DX11_IMAGE_INFO_DEFINED
typedef struct _D3DX11_IMAGE_INFO {
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT ArraySize;
    UINT MipLevels;
    UINT MiscFlags;
    DXGI_FORMAT Format;
    D3D11_RESOURCE_DIMENSION ResourceDimension;
    D3D11_USAGE Usage;
    UINT BindFlags;
    UINT CPUAccessFlags;
    UINT MiscFlags2;
} D3DX11_IMAGE_INFO;
#endif

// Function declarations (link against d3dx11.lib / Effects11)
#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI D3DX11CompileEffectFromFileA(
    LPCSTR pFileName,
    const D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    DWORD Flags,
    DWORD FXFlags,
    ID3D11Device* pDevice,
    ID3DX11Effect** ppEffect,
    ID3DBlob** ppErrorMsgs,
    void* pPool);

HRESULT WINAPI D3DX11CompileEffectFromFileW(
    LPCWSTR pFileName,
    const D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    DWORD Flags,
    DWORD FXFlags,
    ID3D11Device* pDevice,
    ID3DX11Effect** ppEffect,
    ID3DBlob** ppErrorMsgs,
    void* pPool);

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
    void* pPool);

HRESULT WINAPI D3DX11CreateShaderResourceViewFromFileA(
    ID3D11Device* pDevice,
    LPCSTR pSrcFile,
    const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc,
    ID3D11DeviceContext* pContext,
    ID3D11ShaderResourceView** ppShaderResourceView,
    void* pPool);

#ifdef __cplusplus
}
#endif

#ifndef D3DX11_EFFECT_FLAG
#define D3DX11_EFFECT_FLAG 0
#endif
