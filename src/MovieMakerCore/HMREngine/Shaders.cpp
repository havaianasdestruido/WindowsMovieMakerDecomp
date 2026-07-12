// Shaders.cpp - Shader implementations

#include "Shaders.h"
#include <fstream>

namespace HMREngine
{
    // --- ShaderBase ---

    HRESULT ShaderBase::LoadEffect(ID3D11Device* pDev, const char* path)
    {
        if (!pDev || !path) return E_POINTER;

        DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        CComPtr<ID3DX11Effect> effect;
        HRESULT hr = D3DX11CompileEffectFromFileA(
            path, nullptr, nullptr, flags, 0, pDev, &effect, nullptr);

        if (FAILED(hr)) return hr;

        m_effect = effect;
        return S_OK;
    }

    void ShaderBase::SetTechnique(const char* name)
    {
        if (m_effect)
        {
            m_technique = m_effect->GetTechniqueByName(name);
            if (m_technique)
            {
                D3DX11_TECHNIQUE_DESC td;
                m_technique->GetDesc(&td);
                if (td.Passes > 0)
                {
                    m_pass = m_technique->GetPassByIndex(0);
                }
            }
        }
    }

    HRESULT ShaderBase::Initialize(ID3D11Device* pDev, const char* effectPath)
    {
        HRESULT hr = LoadEffect(pDev, effectPath);
        if (FAILED(hr)) return hr;

        SetTechnique("Main");
        return S_OK;
    }

    void ShaderBase::Begin(ID3D11DeviceContext* pCtx)
    {
        if (m_pass) m_pass->Apply(0, pCtx);
    }

    void ShaderBase::End(ID3D11DeviceContext* pCtx)
    {
    }

    // --- GridShader ---
    void GridShader::SetConstants(ID3D11DeviceContext* pCtx, const GridConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_grid");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void GridShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2);
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- FadeShader ---
    void FadeShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void FadeShader::SetConstants(ID3D11DeviceContext* pCtx, const FadeConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_fade");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void FadeShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- DissolveShader ---
    void DissolveShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void DissolveShader::SetNoiseTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_noiseTexture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void DissolveShader::SetConstants(ID3D11DeviceContext* pCtx, const DissolveConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_dissolve");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void DissolveShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- ChannelMixerShader ---
    void ChannelMixerShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void ChannelMixerShader::SetConstants(ID3D11DeviceContext* pCtx, const ChannelMixerConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_channelMixer");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void ChannelMixerShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- BrightnessShader ---
    void BrightnessShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void BrightnessShader::SetConstants(ID3D11DeviceContext* pCtx, const BrightnessConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_brightness");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void BrightnessShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- BlurShader ---
    void BlurShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void BlurShader::SetConstants(ID3D11DeviceContext* pCtx, const BlurConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_blur");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void BlurShader::DrawHorizontal(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        SetTechnique("BlurH");
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    void BlurShader::DrawVertical(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        SetTechnique("BlurV");
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- PosterizeShader ---
    void PosterizeShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void PosterizeShader::SetConstants(ID3D11DeviceContext* pCtx, const PosterizeConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_posterize");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void PosterizeShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- PixelateShader ---
    void PixelateShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void PixelateShader::SetConstants(ID3D11DeviceContext* pCtx, const PixelateConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_pixelate");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void PixelateShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- RippleShader ---
    void RippleShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void RippleShader::SetConstants(ID3D11DeviceContext* pCtx, const RippleConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_ripple");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void RippleShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- HueShader ---
    void HueShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void HueShader::SetConstants(ID3D11DeviceContext* pCtx, const HueConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_hue");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void HueShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- EdgeDetectionShader ---
    void EdgeDetectionShader::SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_texture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void EdgeDetectionShader::SetConstants(ID3D11DeviceContext* pCtx, const EdgeDetectionConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_edgeDetect");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void EdgeDetectionShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- TextShader ---
    void TextShader::SetFontTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_fontTexture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void TextShader::SetConstants(ID3D11DeviceContext* pCtx, const TextConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_text");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void TextShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2 + sizeof(Vec2); // pos + uv + color offset
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- ScrollingTextShader ---
    void ScrollingTextShader::SetFontTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_fontTexture")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void ScrollingTextShader::SetConstants(ID3D11DeviceContext* pCtx, const ScrollingTextConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_scrollingText");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void ScrollingTextShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2 + sizeof(Vec2);
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

    // --- WipeShader ---
    void WipeShader::SetTextureFrom(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_textureFrom")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void WipeShader::SetTextureTo(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv)
    {
        if (m_effect)
        {
            ID3DX11EffectShaderResourceVariable* var = m_effect->GetVariableByName("g_textureTo")->AsShaderResource();
            if (var) var->SetResource(srv);
        }
    }

    void WipeShader::SetConstants(ID3D11DeviceContext* pCtx, const WipeConstants& c)
    {
        if (m_effect)
        {
            ID3DX11EffectVariable* var = m_effect->GetVariableByName("g_wipe");
            if (var) var->SetRawValue(&c, 0, sizeof(c));
        }
    }

    void WipeShader::Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount)
    {
        UINT stride = sizeof(Vec2) * 2;
        UINT offset = 0;
        pCtx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        Begin(pCtx);
        pCtx->Draw(vertexCount, 0);
    }

} // namespace HMREngine
