#include "pch.h"
// EffectResourceDX.cpp - HLSL effect wrapper implementation

#include "EffectResourceDX.h"
#include "../../Resources/ResourceIds.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace HMREngine
{
namespace DX
{

// ============================================================================
// Helper: Get variable by name
// ============================================================================
template<typename T>
static HRESULT GetEffectVariable(ID3DX11Effect* effect, const char* name, CComPtr<T>& var)
{
    if (!effect) return E_FAIL;
    var.Attach(static_cast<T*>(effect->GetVariableByName(name)));
    return var ? S_OK : E_FAIL;
}

// ============================================================================
// EffectResourceDX
// ============================================================================
EffectResourceDX::EffectResourceDX() = default;
EffectResourceDX::~EffectResourceDX() { Release(); }

HRESULT EffectResourceDX::LoadFromFile(const std::wstring& filePath)
{
    m_filePath = filePath;
    m_loaded = false;

    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef NDEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    CComPtr<ID3DBlob> shaderBlob;
    CComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DX11CompileEffectFromFileW(filePath.c_str(), nullptr, nullptr,
        flags, 0, m_device, &m_effect, &errorBlob, nullptr);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        return hr;
    }

    m_loaded = true;
    return S_OK;
}

HRESULT EffectResourceDX::LoadFromMemory(const void* data, UINT dataSize)
{
    if (!data || dataSize == 0) return E_INVALIDARG;
    m_loaded = false;

    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef NDEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    CComPtr<ID3DBlob> shaderBlob;
    CComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DX11CompileEffectFromMemory(data, dataSize, nullptr, nullptr, nullptr,
        flags, 0, m_device, &m_effect, &errorBlob, nullptr);

    if (FAILED(hr) && errorBlob)
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());

    if (SUCCEEDED(hr)) m_loaded = true;
    return hr;
}

HRESULT EffectResourceDX::LoadFromResource(HMODULE hModule, UINT resourceId)
{
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(resourceId), L"EFFECT");
    if (!hRes) return HRESULT_FROM_WIN32(GetLastError());

    HGLOBAL hGlobal = LoadResource(hModule, hRes);
    if (!hGlobal) return HRESULT_FROM_WIN32(GetLastError());

    DWORD size = SizeofResource(hModule, hRes);
    const void* data = LockResource(hGlobal);

    HRESULT hr = LoadFromMemory(data, size);

    UnlockResource(hGlobal);
    FreeResource(hGlobal);
    return hr;
}

void EffectResourceDX::Release()
{
    m_currentPass.Release();
    m_technique.Release();
    m_effect.Release();
    m_loaded = false;
}

HRESULT EffectResourceDX::SetTechnique(const char* techniqueName)
{
    if (!m_effect) return E_FAIL;

    m_technique = m_effect->GetTechniqueByName(techniqueName);
    return m_technique ? S_OK : E_FAIL;
}

HRESULT EffectResourceDX::BeginPass(UINT passIndex)
{
    if (!m_technique) return E_FAIL;

    D3DX11_TECHNIQUE_DESC techDesc;
    HRESULT hr = m_technique->GetDesc(&techDesc);
    if (FAILED(hr)) return hr;

    if (passIndex >= techDesc.Passes) return E_INVALIDARG;

    m_currentPass = m_technique->GetPassByIndex(passIndex);
    if (!m_currentPass) return E_FAIL;

    return m_currentPass->Apply(0, m_context);
}

HRESULT EffectResourceDX::EndPass()
{
    m_currentPass.Release();
    return S_OK;
}

void EffectResourceDX::SetFloat(const char* name, float value)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectScalarVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsScalar());
    if (var) var->SetFloat(value);
}

void EffectResourceDX::SetFloat2(const char* name, const Vec2& value)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectVectorVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsVector());
    if (var) var->SetFloatVector(reinterpret_cast<const float*>(&value));
}

void EffectResourceDX::SetFloat3(const char* name, const Vec3& value)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectVectorVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsVector());
    if (var) var->SetFloatVector(reinterpret_cast<const float*>(&value));
}

void EffectResourceDX::SetFloat4(const char* name, const Vec4& value)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectVectorVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsVector());
    if (var) var->SetFloatVector(reinterpret_cast<const float*>(&value));
}

void EffectResourceDX::SetMatrix(const char* name, const Matrix4f& value)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectMatrixVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsMatrix());
    if (var) var->SetMatrix(reinterpret_cast<const float*>(&value));
}

void EffectResourceDX::SetTexture(const char* name, ID3D11ShaderResourceView* srv)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectShaderResourceVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsShaderResource());
    if (var) var->SetResource(srv);
}

void EffectResourceDX::SetInt(const char* name, int value)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectScalarVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsScalar());
    if (var) var->SetInt(value);
}

void EffectResourceDX::SetBool(const char* name, bool value)
{
    if (!m_effect) return;
    CComPtr<ID3DX11EffectScalarVariable> var;
    var.Attach(m_effect->GetVariableByName(name)->AsScalar());
    if (var) var->SetBool(value ? TRUE : FALSE);
}

// ============================================================================
// CommonEffectResourceDX
// ============================================================================
CommonEffectResourceDX::CommonEffectResourceDX() = default;
CommonEffectResourceDX::~CommonEffectResourceDX() = default;

HRESULT CommonEffectResourceDX::LoadCommonEffect(ID3D11Device* dev)
{
    m_device = dev;
    return LoadFromResource(nullptr, IDR_COMMON_EFFECT);
}

void CommonEffectResourceDX::SetWorldViewProj(const Matrix4f& wvp)
{
    if (m_wvpVar) m_wvpVar->SetMatrix(reinterpret_cast<const float*>(&wvp));
}

void CommonEffectResourceDX::SetWorld(const Matrix4f& world)
{
    if (m_worldVar) m_worldVar->SetMatrix(reinterpret_cast<const float*>(&world));
}

void CommonEffectResourceDX::SetView(const Matrix4f& view)
{
    if (m_viewVar) m_viewVar->SetMatrix(reinterpret_cast<const float*>(&view));
}

void CommonEffectResourceDX::SetProjection(const Matrix4f& proj)
{
    if (m_projVar) m_projVar->SetMatrix(reinterpret_cast<const float*>(&proj));
}

void CommonEffectResourceDX::SetTime(float time)
{
    if (m_timeVar) m_timeVar->SetFloat(time);
}

void CommonEffectResourceDX::SetResolution(float width, float height)
{
    if (m_resolutionVar)
    {
        Vec4 res(width, height, 1.0f / width, 1.0f / height);
        m_resolutionVar->SetFloatVector(reinterpret_cast<const float*>(&res));
    }
}

// ============================================================================
// SimpleEffectResourceBase
// ============================================================================
SimpleEffectResourceBase::SimpleEffectResourceBase() = default;
SimpleEffectResourceBase::~SimpleEffectResourceBase() = default;

HRESULT SimpleEffectResourceBase::Initialize(ID3D11Device* dev, const char* effectFile)
{
    m_device = dev;
    int len = MultiByteToWideChar(CP_ACP, 0, effectFile, -1, nullptr, 0);
    std::wstring wPath(len, 0);
    MultiByteToWideChar(CP_ACP, 0, effectFile, -1, &wPath[0], len);
    wPath.resize(len - 1);
    return LoadFromFile(wPath);
}

void SimpleEffectResourceBase::Apply(ID3D11DeviceContext* ctx)
{
    if (m_effect && m_technique)
        BeginPass(0);
}

// ============================================================================
// BlurEffectResource
// ============================================================================
BlurEffectResource::BlurEffectResource() = default;
BlurEffectResource::~BlurEffectResource() = default;

HRESULT BlurEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Blur.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Blur");
    GetEffectVariable(m_effect, "g_direction", m_directionVar);
    GetEffectVariable(m_effect, "g_texelSize", m_texelSizeVar);
    GetEffectVariable(m_effect, "g_blurRadius", m_blurRadiusVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void BlurEffectResource::SetDirection(const Vec2& dir)
{
    if (m_directionVar) m_directionVar->SetFloatVector(reinterpret_cast<const float*>(&dir));
}

void BlurEffectResource::SetTexelSize(const Vec2& size)
{
    if (m_texelSizeVar) m_texelSizeVar->SetFloatVector(reinterpret_cast<const float*>(&size));
}

void BlurEffectResource::SetBlurRadius(float radius)
{
    if (m_blurRadiusVar) m_blurRadiusVar->SetFloat(radius);
}

void BlurEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv)
{
    if (m_inputTextureVar) m_inputTextureVar->SetResource(srv);
}

// ============================================================================
// BrightnessEffectResource
// ============================================================================
BrightnessEffectResource::BrightnessEffectResource() = default;
BrightnessEffectResource::~BrightnessEffectResource() = default;

HRESULT BrightnessEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Brightness.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Brightness");
    GetEffectVariable(m_effect, "g_brightness", m_brightnessVar);
    GetEffectVariable(m_effect, "g_contrast", m_contrastVar);
    GetEffectVariable(m_effect, "g_gamma", m_gammaVar);
    GetEffectVariable(m_effect, "g_saturation", m_saturationVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void BrightnessEffectResource::SetBrightness(float brightness)
{
    if (m_brightnessVar) m_brightnessVar->SetFloat(brightness);
}

void BrightnessEffectResource::SetContrast(float contrast)
{
    if (m_contrastVar) m_contrastVar->SetFloat(contrast);
}

void BrightnessEffectResource::SetGamma(float gamma)
{
    if (m_gammaVar) m_gammaVar->SetFloat(gamma);
}

void BrightnessEffectResource::SetSaturation(float saturation)
{
    if (m_saturationVar) m_saturationVar->SetFloat(saturation);
}

void BrightnessEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv)
{
    if (m_inputTextureVar) m_inputTextureVar->SetResource(srv);
}

// ============================================================================
// ChannelMixerEffectResource
// ============================================================================
ChannelMixerEffectResource::ChannelMixerEffectResource() = default;
ChannelMixerEffectResource::~ChannelMixerEffectResource() = default;

HRESULT ChannelMixerEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "ChannelMixer.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("ChannelMixer");
    GetEffectVariable(m_effect, "g_redVector", m_redVectorVar);
    GetEffectVariable(m_effect, "g_greenVector", m_greenVectorVar);
    GetEffectVariable(m_effect, "g_blueVector", m_blueVectorVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void ChannelMixerEffectResource::SetRedVector(const Vec3& v)
{
    if (m_redVectorVar) m_redVectorVar->SetFloatVector(reinterpret_cast<const float*>(&v));
}

void ChannelMixerEffectResource::SetGreenVector(const Vec3& v)
{
    if (m_greenVectorVar) m_greenVectorVar->SetFloatVector(reinterpret_cast<const float*>(&v));
}

void ChannelMixerEffectResource::SetBlueVector(const Vec3& v)
{
    if (m_blueVectorVar) m_blueVectorVar->SetFloatVector(reinterpret_cast<const float*>(&v));
}

void ChannelMixerEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv)
{
    if (m_inputTextureVar) m_inputTextureVar->SetResource(srv);
}

// ============================================================================
// DissolveEffectResource
// ============================================================================
DissolveEffectResource::DissolveEffectResource() = default;
DissolveEffectResource::~DissolveEffectResource() = default;

HRESULT DissolveEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Dissolve.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Dissolve");
    GetEffectVariable(m_effect, "g_threshold", m_thresholdVar);
    GetEffectVariable(m_effect, "g_edgeWidth", m_edgeWidthVar);
    GetEffectVariable(m_effect, "g_edgeColor", m_edgeColorVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    GetEffectVariable(m_effect, "g_noiseTexture", m_noiseTextureVar);
    return S_OK;
}

void DissolveEffectResource::SetThreshold(float threshold)
{
    if (m_thresholdVar) m_thresholdVar->SetFloat(threshold);
}

void DissolveEffectResource::SetEdgeWidth(float width)
{
    if (m_edgeWidthVar) m_edgeWidthVar->SetFloat(width);
}

void DissolveEffectResource::SetEdgeColor(const Rgb& color)
{
    if (m_edgeColorVar)
    {
        Vec4 c(color.r, color.g, color.b, 1.0f);
        m_edgeColorVar->SetFloatVector(reinterpret_cast<const float*>(&c));
    }
}

void DissolveEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv)
{
    if (m_inputTextureVar) m_inputTextureVar->SetResource(srv);
}

void DissolveEffectResource::SetNoiseTexture(ID3D11ShaderResourceView* srv)
{
    if (m_noiseTextureVar) m_noiseTextureVar->SetResource(srv);
}

// ============================================================================
// AnimatedEffectResourceBase
// ============================================================================
AnimatedEffectResourceBase::AnimatedEffectResourceBase() = default;
AnimatedEffectResourceBase::~AnimatedEffectResourceBase() = default;

void AnimatedEffectResourceBase::SetTime(float time)
{
    if (m_timeVar) m_timeVar->SetFloat(time);
}

void AnimatedEffectResourceBase::SetProgress(float progress)
{
    if (m_progressVar) m_progressVar->SetFloat(progress);
}

// ============================================================================
// EdgeDetectionEffectResource
// ============================================================================
EdgeDetectionEffectResource::EdgeDetectionEffectResource() = default;
EdgeDetectionEffectResource::~EdgeDetectionEffectResource() = default;

HRESULT EdgeDetectionEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "EdgeDetection.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("EdgeDetection");
    GetEffectVariable(m_effect, "g_texelSize", m_texelSizeVar);
    GetEffectVariable(m_effect, "g_threshold", m_thresholdVar);
    GetEffectVariable(m_effect, "g_edgeColor", m_edgeColorVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void EdgeDetectionEffectResource::SetTexelSize(const Vec2& size)
{
    if (m_texelSizeVar) m_texelSizeVar->SetFloatVector(reinterpret_cast<const float*>(&size));
}

void EdgeDetectionEffectResource::SetThreshold(float threshold)
{
    if (m_thresholdVar) m_thresholdVar->SetFloat(threshold);
}

void EdgeDetectionEffectResource::SetEdgeColor(const Rgb& color)
{
    if (m_edgeColorVar)
    {
        Vec4 c(color.r, color.g, color.b, 1.0f);
        m_edgeColorVar->SetFloatVector(reinterpret_cast<const float*>(&c));
    }
}

void EdgeDetectionEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv)
{
    if (m_inputTextureVar) m_inputTextureVar->SetResource(srv);
}

// ============================================================================
// FadeEffectResource
// ============================================================================
FadeEffectResource::FadeEffectResource() = default;
FadeEffectResource::~FadeEffectResource() = default;

HRESULT FadeEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Fade.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Fade");
    GetEffectVariable(m_effect, "g_alpha", m_alphaVar);
    GetEffectVariable(m_effect, "g_fadeDirection", m_fadeDirectionVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    GetEffectVariable(m_effect, "g_time", m_timeVar);
    return S_OK;
}

void FadeEffectResource::SetAlpha(float alpha)
{
    if (m_alphaVar) m_alphaVar->SetFloat(alpha);
}

void FadeEffectResource::SetFadeDirection(float dir)
{
    if (m_fadeDirectionVar) m_fadeDirectionVar->SetFloat(dir);
}

void FadeEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv)
{
    if (m_inputTextureVar) m_inputTextureVar->SetResource(srv);
}

// ============================================================================
// GridEffectResource
// ============================================================================
GridEffectResource::GridEffectResource() = default;
GridEffectResource::~GridEffectResource() = default;

HRESULT GridEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Grid.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Grid");
    GetEffectVariable(m_effect, "g_gridMin", m_gridMinVar);
    GetEffectVariable(m_effect, "g_gridMax", m_gridMaxVar);
    GetEffectVariable(m_effect, "g_divisions", m_divisionsVar);
    GetEffectVariable(m_effect, "g_lineColor", m_lineColorVar);
    GetEffectVariable(m_effect, "g_lineWidth", m_lineWidthVar);
    GetEffectVariable(m_effect, "g_showOrigin", m_showOriginVar);
    GetEffectVariable(m_effect, "g_originColor", m_originColorVar);
    return S_OK;
}

void GridEffectResource::SetGridMin(const Vec2& min) { if (m_gridMinVar) m_gridMinVar->SetFloatVector(reinterpret_cast<const float*>(&min)); }
void GridEffectResource::SetGridMax(const Vec2& max) { if (m_gridMaxVar) m_gridMaxVar->SetFloatVector(reinterpret_cast<const float*>(&max)); }
void GridEffectResource::SetDivisions(const Vec2& div) { if (m_divisionsVar) m_divisionsVar->SetFloatVector(reinterpret_cast<const float*>(&div)); }
void GridEffectResource::SetLineColor(const Rgb& color) { if (m_lineColorVar) { Vec4 c(color.r, color.g, color.b, 1.0f); m_lineColorVar->SetFloatVector(reinterpret_cast<const float*>(&c)); } }
void GridEffectResource::SetLineWidth(float width) { if (m_lineWidthVar) m_lineWidthVar->SetFloat(width); }
void GridEffectResource::SetShowOrigin(bool show) { if (m_showOriginVar) m_showOriginVar->SetBool(show); }
void GridEffectResource::SetOriginColor(const Rgb& color) { if (m_originColorVar) { Vec4 c(color.r, color.g, color.b, 1.0f); m_originColorVar->SetFloatVector(reinterpret_cast<const float*>(&c)); } }

// ============================================================================
// HueEffectResource
// ============================================================================
HueEffectResource::HueEffectResource() = default;
HueEffectResource::~HueEffectResource() = default;

HRESULT HueEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "HueShift.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("HueShift");
    GetEffectVariable(m_effect, "g_hueShift", m_hueShiftVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void HueEffectResource::SetHueShift(float shift) { if (m_hueShiftVar) m_hueShiftVar->SetFloat(shift); }
void HueEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv) { if (m_inputTextureVar) m_inputTextureVar->SetResource(srv); }

// ============================================================================
// PosterizeEffectResource
// ============================================================================
PosterizeEffectResource::PosterizeEffectResource() = default;
PosterizeEffectResource::~PosterizeEffectResource() = default;

HRESULT PosterizeEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Posterize.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Posterize");
    GetEffectVariable(m_effect, "g_numLevels", m_numLevelsVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void PosterizeEffectResource::SetNumLevels(float levels) { if (m_numLevelsVar) m_numLevelsVar->SetFloat(levels); }
void PosterizeEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv) { if (m_inputTextureVar) m_inputTextureVar->SetResource(srv); }

// ============================================================================
// PixelateEffectResource
// ============================================================================
PixelateEffectResource::PixelateEffectResource() = default;
PixelateEffectResource::~PixelateEffectResource() = default;

HRESULT PixelateEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Pixelate.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Pixelate");
    GetEffectVariable(m_effect, "g_pixelSize", m_pixelSizeVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void PixelateEffectResource::SetPixelSize(const Vec2& size) { if (m_pixelSizeVar) m_pixelSizeVar->SetFloatVector(reinterpret_cast<const float*>(&size)); }
void PixelateEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv) { if (m_inputTextureVar) m_inputTextureVar->SetResource(srv); }

// ============================================================================
// RippleEffectResource
// ============================================================================
RippleEffectResource::RippleEffectResource() = default;
RippleEffectResource::~RippleEffectResource() = default;

HRESULT RippleEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Ripple.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Ripple");
    GetEffectVariable(m_effect, "g_amplitude", m_amplitudeVar);
    GetEffectVariable(m_effect, "g_frequency", m_frequencyVar);
    GetEffectVariable(m_effect, "g_speed", m_speedVar);
    GetEffectVariable(m_effect, "g_time", m_timeVar);
    GetEffectVariable(m_effect, "g_inputTexture", m_inputTextureVar);
    return S_OK;
}

void RippleEffectResource::SetAmplitude(float amp) { if (m_amplitudeVar) m_amplitudeVar->SetFloat(amp); }
void RippleEffectResource::SetFrequency(float freq) { if (m_frequencyVar) m_frequencyVar->SetFloat(freq); }
void RippleEffectResource::SetSpeed(float speed) { if (m_speedVar) m_speedVar->SetFloat(speed); }
void RippleEffectResource::SetInputTexture(ID3D11ShaderResourceView* srv) { if (m_inputTextureVar) m_inputTextureVar->SetResource(srv); }

// ============================================================================
// ScrollingTextEffectResourceDX
// ============================================================================
ScrollingTextEffectResourceDX::ScrollingTextEffectResourceDX() = default;
ScrollingTextEffectResourceDX::~ScrollingTextEffectResourceDX() = default;

HRESULT ScrollingTextEffectResourceDX::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "ScrollingText.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("ScrollingText");
    GetEffectVariable(m_effect, "g_position", m_positionVar);
    GetEffectVariable(m_effect, "g_textSize", m_textSizeVar);
    GetEffectVariable(m_effect, "g_textColor", m_textColorVar);
    GetEffectVariable(m_effect, "g_scrollOffset", m_scrollOffsetVar);
    GetEffectVariable(m_effect, "g_scrollSpeed", m_scrollSpeedVar);
    GetEffectVariable(m_effect, "g_fontTexture", m_fontTextureVar);
    return S_OK;
}

void ScrollingTextEffectResourceDX::SetPosition(const Vec2& pos) { if (m_positionVar) m_positionVar->SetFloatVector(reinterpret_cast<const float*>(&pos)); }
void ScrollingTextEffectResourceDX::SetTextSize(const Vec2& size) { if (m_textSizeVar) m_textSizeVar->SetFloatVector(reinterpret_cast<const float*>(&size)); }
void ScrollingTextEffectResourceDX::SetTextColor(const Rgba& color) { if (m_textColorVar) m_textColorVar->SetFloatVector(reinterpret_cast<const float*>(&color)); }
void ScrollingTextEffectResourceDX::SetScrollOffset(float offset) { if (m_scrollOffsetVar) m_scrollOffsetVar->SetFloat(offset); }
void ScrollingTextEffectResourceDX::SetScrollSpeed(float speed) { if (m_scrollSpeedVar) m_scrollSpeedVar->SetFloat(speed); }
void ScrollingTextEffectResourceDX::SetFontTexture(ID3D11ShaderResourceView* srv) { if (m_fontTextureVar) m_fontTextureVar->SetResource(srv); }

// ============================================================================
// TextEffectResourceDX
// ============================================================================
TextEffectResourceDX::TextEffectResourceDX() = default;
TextEffectResourceDX::~TextEffectResourceDX() = default;

HRESULT TextEffectResourceDX::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Text.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Text");
    GetEffectVariable(m_effect, "g_position", m_positionVar);
    GetEffectVariable(m_effect, "g_textSize", m_textSizeVar);
    GetEffectVariable(m_effect, "g_textColor", m_textColorVar);
    GetEffectVariable(m_effect, "g_rotation", m_rotationVar);
    GetEffectVariable(m_effect, "g_fontTexture", m_fontTextureVar);
    return S_OK;
}

void TextEffectResourceDX::SetPosition(const Vec2& pos) { if (m_positionVar) m_positionVar->SetFloatVector(reinterpret_cast<const float*>(&pos)); }
void TextEffectResourceDX::SetTextSize(const Vec2& size) { if (m_textSizeVar) m_textSizeVar->SetFloatVector(reinterpret_cast<const float*>(&size)); }
void TextEffectResourceDX::SetTextColor(const Rgba& color) { if (m_textColorVar) m_textColorVar->SetFloatVector(reinterpret_cast<const float*>(&color)); }
void TextEffectResourceDX::SetRotation(float rotation) { if (m_rotationVar) m_rotationVar->SetFloat(rotation); }
void TextEffectResourceDX::SetFontTexture(ID3D11ShaderResourceView* srv) { if (m_fontTextureVar) m_fontTextureVar->SetResource(srv); }

// ============================================================================
// WipeEffectResource
// ============================================================================
WipeEffectResource::WipeEffectResource() = default;
WipeEffectResource::~WipeEffectResource() = default;

HRESULT WipeEffectResource::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Wipe.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Wipe");
    GetEffectVariable(m_effect, "g_progress", m_progressVar);
    GetEffectVariable(m_effect, "g_direction", m_directionVar);
    GetEffectVariable(m_effect, "g_softness", m_softnessVar);
    GetEffectVariable(m_effect, "g_textureFrom", m_textureFromVar);
    GetEffectVariable(m_effect, "g_textureTo", m_textureToVar);
    GetEffectVariable(m_effect, "g_time", m_timeVar);
    return S_OK;
}

void WipeEffectResource::SetProgress(float progress) { if (m_progressVar) m_progressVar->SetFloat(progress); }
void WipeEffectResource::SetDirection(float direction) { if (m_directionVar) m_directionVar->SetFloat(direction); }
void WipeEffectResource::SetSoftness(float softness) { if (m_softnessVar) m_softnessVar->SetFloat(softness); }
void WipeEffectResource::SetTextureFrom(ID3D11ShaderResourceView* srv) { if (m_textureFromVar) m_textureFromVar->SetResource(srv); }
void WipeEffectResource::SetTextureTo(ID3D11ShaderResourceView* srv) { if (m_textureToVar) m_textureToVar->SetResource(srv); }

// ============================================================================
// DefaultEffectResourceDX
// ============================================================================
DefaultEffectResourceDX::DefaultEffectResourceDX() = default;
DefaultEffectResourceDX::~DefaultEffectResourceDX() = default;

HRESULT DefaultEffectResourceDX::Initialize(ID3D11Device* dev)
{
    HRESULT hr = SimpleEffectResourceBase::Initialize(dev, "Default.fx");
    if (FAILED(hr)) return hr;

    SetTechnique("Default");
    GetEffectVariable(m_effect, "g_texture", m_textureVar);
    GetEffectVariable(m_effect, "g_transform", m_transformVar);
    return S_OK;
}

void DefaultEffectResourceDX::SetTexture(ID3D11ShaderResourceView* srv)
{
    if (m_textureVar) m_textureVar->SetResource(srv);
}

void DefaultEffectResourceDX::SetTransform(const Matrix4f& transform)
{
    if (m_transformVar) m_transformVar->SetMatrix(reinterpret_cast<const float*>(&transform));
}

} // namespace DX
} // namespace HMREngine
