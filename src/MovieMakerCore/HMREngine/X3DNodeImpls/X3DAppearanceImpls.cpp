#include "pch.h"
// X3DAppearanceImpls.cpp - Appearance/texture/material/shader bridge implementation

#include "X3DAppearanceImpls.h"

namespace HMREngine
{

// ============================================================================
// X3DTextureNodeImpl
// ============================================================================
X3DTextureNodeImpl::X3DTextureNodeImpl() = default;
X3DTextureNodeImpl::~X3DTextureNodeImpl() { Shutdown(); }

HRESULT X3DTextureNodeImpl::Initialize(X3DTextureNode* node)
{
    m_textureNode = node;
    return X3DChildNodeImpl::Initialize(node);
}

void X3DTextureNodeImpl::Shutdown()
{
    m_textureNode = nullptr;
    X3DChildNodeImpl::Shutdown();
}

HRESULT X3DTextureNodeImpl::GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV)
{
    if (!m_textureNode || !ppSRV) return E_POINTER;
    return m_textureNode->GetTexture(dev, ppSRV);
}

void X3DTextureNodeImpl::Update(double time)
{
    if (m_textureNode) m_textureNode->Update(time);
}

HRESULT X3DTextureNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DTexture2DNodeImpl
// ============================================================================
X3DTexture2DNodeImpl::X3DTexture2DNodeImpl() = default;
X3DTexture2DNodeImpl::~X3DTexture2DNodeImpl() { Shutdown(); }

HRESULT X3DTexture2DNodeImpl::Initialize(X3DTextureNode* node)
{
    return X3DTextureNodeImpl::Initialize(node);
}

void X3DTexture2DNodeImpl::Shutdown()
{
    X3DTextureNodeImpl::Shutdown();
}

HRESULT X3DTexture2DNodeImpl::GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV)
{
    return X3DTextureNodeImpl::GetTexture(dev, ppSRV);
}

HRESULT X3DTexture2DNodeImpl::SetupFields()
{
    return X3DTextureNodeImpl::SetupFields();
}

// ============================================================================
// X3DUrlObjectImpl
// ============================================================================
X3DUrlObjectImpl::X3DUrlObjectImpl() = default;
X3DUrlObjectImpl::~X3DUrlObjectImpl() { Shutdown(); }

HRESULT X3DUrlObjectImpl::Initialize(X3DNode* node)
{
    return X3DNodeImpl::Initialize(node);
}

void X3DUrlObjectImpl::Shutdown()
{
    m_url.clear();
    X3DNodeImpl::Shutdown();
}

HRESULT X3DUrlObjectImpl::ResolveUrl(std::wstring& outPath) const
{
    if (m_url.empty()) return E_FAIL;

    int len = MultiByteToWideChar(CP_ACP, 0, m_url.c_str(), -1, nullptr, 0);
    outPath.resize(len);
    MultiByteToWideChar(CP_ACP, 0, m_url.c_str(), -1, &outPath[0], len);
    outPath.resize(len - 1);
    return S_OK;
}

HRESULT X3DUrlObjectImpl::SetupFields()
{
    return X3DNodeImpl::SetupFields();
}

// ============================================================================
// X3DMaterialNodeImpl
// ============================================================================
X3DMaterialNodeImpl::X3DMaterialNodeImpl() = default;
X3DMaterialNodeImpl::~X3DMaterialNodeImpl() { Shutdown(); }

HRESULT X3DMaterialNodeImpl::Initialize(X3DMaterialNode* node)
{
    m_materialNode = node;
    if (node)
    {
        MaterialNode* mat = static_cast<MaterialNode*>(node);
        m_diffuseColor = mat->m_diffuseColor;
        m_specularColor = mat->m_specularColor;
        m_emissiveColor = mat->m_emissiveColor;
        m_ambientColor = mat->m_ambientColor;
        m_shininess = mat->m_shininess;
        m_transparency = mat->m_transparency;
    }
    return X3DChildNodeImpl::Initialize(node);
}

void X3DMaterialNodeImpl::Shutdown()
{
    m_materialNode = nullptr;
    X3DChildNodeImpl::Shutdown();
}

HRESULT X3DMaterialNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DAppearanceNodeImpl
// ============================================================================
X3DAppearanceNodeImpl::X3DAppearanceNodeImpl() = default;
X3DAppearanceNodeImpl::~X3DAppearanceNodeImpl() { Shutdown(); }

HRESULT X3DAppearanceNodeImpl::Initialize(X3DChildNode* node)
{
    return X3DChildNodeImpl::Initialize(node);
}

void X3DAppearanceNodeImpl::Shutdown()
{
    m_texture = nullptr;
    m_material = nullptr;
    m_textureTransform = nullptr;
    X3DChildNodeImpl::Shutdown();
}

void X3DAppearanceNodeImpl::SetTexture(X3DTextureNodeImpl* tex)
{
    m_texture = tex;
    MarkDirty();
}

void X3DAppearanceNodeImpl::SetMaterial(X3DMaterialNodeImpl* mat)
{
    m_material = mat;
    MarkDirty();
}

void X3DAppearanceNodeImpl::SetTextureTransform(TextureTransformContainerImpl* tt)
{
    m_textureTransform = tt;
    MarkDirty();
}

HRESULT X3DAppearanceNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// TextureTransformContainerImpl
// ============================================================================
TextureTransformContainerImpl::TextureTransformContainerImpl() = default;
TextureTransformContainerImpl::~TextureTransformContainerImpl() = default;

HRESULT TextureTransformContainerImpl::Initialize(X3DChildNode* node)
{
    return X3DNodeImpl::Initialize(node);
}

void TextureTransformContainerImpl::Shutdown()
{
    X3DNodeImpl::Shutdown();
}

Matrix4f TextureTransformContainerImpl::GetMatrix() const
{
    Matrix4f m = Matrix4f::Identity();
    m._11 = m_scale.x * cosf(m_rotation);
    m._12 = m_scale.x * sinf(m_rotation);
    m._21 = -m_scale.y * sinf(m_rotation);
    m._22 = m_scale.y * cosf(m_rotation);
    m._41 = m_translation.x;
    m._42 = m_translation.y;
    return m;
}

// ============================================================================
// MotionTextureNodeImpl
// ============================================================================
MotionTextureNodeImpl::MotionTextureNodeImpl() = default;
MotionTextureNodeImpl::~MotionTextureNodeImpl() { Shutdown(); }

HRESULT MotionTextureNodeImpl::Initialize(X3DTextureNode* node)
{
    if (node)
    {
        MotionTextureNode* mtn = static_cast<MotionTextureNode*>(node);
        m_offset = mtn->m_offset;
        m_scale = mtn->m_scale;
    }
    return X3DTextureNodeImpl::Initialize(node);
}

void MotionTextureNodeImpl::Shutdown()
{
    X3DTextureNodeImpl::Shutdown();
}

HRESULT MotionTextureNodeImpl::SetupFields()
{
    return X3DTextureNodeImpl::SetupFields();
}

// ============================================================================
// ShaderSetNodeImpl
// ============================================================================
ShaderSetNodeImpl::ShaderSetNodeImpl() = default;
ShaderSetNodeImpl::~ShaderSetNodeImpl() { Shutdown(); }

HRESULT ShaderSetNodeImpl::Initialize(X3DNode* node)
{
    return X3DNodeImpl::Initialize(node);
}

void ShaderSetNodeImpl::Shutdown()
{
    m_shaders.clear();
    X3DNodeImpl::Shutdown();
}

void ShaderSetNodeImpl::AddShader(X3DShaderNodeImpl* shader)
{
    if (shader) m_shaders.push_back(shader);
}

X3DShaderNodeImpl* ShaderSetNodeImpl::GetShader(UINT index) const
{
    if (index >= m_shaders.size()) return nullptr;
    return m_shaders[index];
}

// ============================================================================
// TexturePropertiesNodeImpl
// ============================================================================
TexturePropertiesNodeImpl::TexturePropertiesNodeImpl() = default;
TexturePropertiesNodeImpl::~TexturePropertiesNodeImpl() = default;

HRESULT TexturePropertiesNodeImpl::Initialize(X3DChildNode* node)
{
    if (node)
    {
        TexturePropertiesNode* tpn = static_cast<TexturePropertiesNode*>(node);
        m_generateMipMaps = tpn->m_generateMipMaps;
        m_magFilter = tpn->m_magFilter;
        m_minFilter = tpn->m_minFilter;
    }
    return X3DNodeImpl::Initialize(node);
}

void TexturePropertiesNodeImpl::Shutdown()
{
    X3DNodeImpl::Shutdown();
}

D3D11_FILTER TexturePropertiesNodeImpl::GetD3DFilter() const
{
    bool linear = (m_magFilter == "LINEAR" || m_minFilter == "LINEAR");
    bool mip = m_generateMipMaps;
    if (linear && mip) return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    if (linear) return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

D3D11_TEXTURE_ADDRESS_MODE TexturePropertiesNodeImpl::GetAddressMode() const
{
    return D3D11_TEXTURE_ADDRESS_WRAP;
}

// ============================================================================
// X3DShaderNodeImpl
// ============================================================================
X3DShaderNodeImpl::X3DShaderNodeImpl() = default;
X3DShaderNodeImpl::~X3DShaderNodeImpl() { Shutdown(); }

HRESULT X3DShaderNodeImpl::Initialize(X3DNode* node)
{
    return X3DNodeImpl::Initialize(node);
}

void X3DShaderNodeImpl::Shutdown()
{
    X3DNodeImpl::Shutdown();
}

HRESULT X3DShaderNodeImpl::SetupFields()
{
    return X3DNodeImpl::SetupFields();
}

// ============================================================================
// X3DAnimatedShaderNodeImpl
// ============================================================================
X3DAnimatedShaderNodeImpl::X3DAnimatedShaderNodeImpl() = default;
X3DAnimatedShaderNodeImpl::~X3DAnimatedShaderNodeImpl() { Shutdown(); }

HRESULT X3DAnimatedShaderNodeImpl::Initialize(X3DNode* node)
{
    return X3DShaderNodeImpl::Initialize(node);
}

void X3DAnimatedShaderNodeImpl::Shutdown()
{
    X3DShaderNodeImpl::Shutdown();
}

HRESULT X3DAnimatedShaderNodeImpl::SetupFields()
{
    return X3DShaderNodeImpl::SetupFields();
}

} // namespace HMREngine
