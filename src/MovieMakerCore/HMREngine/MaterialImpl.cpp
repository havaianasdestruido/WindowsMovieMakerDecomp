// MaterialImpl.cpp - Material/Appearance/Shape/LineProperties/FillProperties

#include "MaterialImpl.h"
#include "TextureImpl.h"
#include "ResourceCache.h"

namespace HMREngine
{
    // MaterialImpl
    MaterialImpl::MaterialImpl() {}
    MaterialImpl::~MaterialImpl() { Shutdown(); }

    HRESULT MaterialImpl::Initialize(MaterialNode* node, ID3D11Device* dev)
    {
        if (!node || !dev) return E_POINTER;

        m_diffuse = node->m_diffuseColor;
        m_specular = node->m_specularColor;
        m_emissive = node->m_emissiveColor;
        m_ambient = node->m_ambientColor;
        m_shininess = node->m_shininess;
        m_transparency = node->m_transparency;

        m_bufferData.diffuseColor = m_diffuse;
        m_bufferData.specularColor = m_specular;
        m_bufferData.emissiveColor = m_emissive;
        m_bufferData.ambientColor = m_ambient;
        m_bufferData.shininess = m_shininess;
        m_bufferData.transparency = m_transparency;

        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(MaterialBuffer);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = &m_bufferData;

        return dev->CreateBuffer(&bd, &initData, &m_constantBuffer);
    }

    void MaterialImpl::Shutdown()
    {
        m_constantBuffer.Release();
    }

    void MaterialImpl::UpdateBuffer(ID3D11DeviceContext* ctx)
    {
        if (m_constantBuffer && ctx)
        {
            ctx->UpdateSubresource(m_constantBuffer, 0, nullptr, &m_bufferData, 0, 0);
        }
    }

    // AppearanceImpl
    AppearanceImpl::AppearanceImpl() {}
    AppearanceImpl::~AppearanceImpl() { Shutdown(); }

    HRESULT AppearanceImpl::Initialize(AppearanceNode* node, ID3D11Device* dev, ResourceCache* cache)
    {
        if (!node || !dev) return E_POINTER;

        if (node->m_material)
        {
            auto* matNode = dynamic_cast<MaterialNode*>(node->m_material.Get());
            if (matNode)
            {
                m_material = new MaterialImpl();
                m_material->Initialize(matNode, dev);
            }
        }

        if (node->m_texture && cache)
        {
            auto* texNode = dynamic_cast<ImageTextureNode*>(node->m_texture.Get());
            if (texNode && !texNode->m_url.empty())
            {
                TextureResource* texRes = cache->GetTexture(texNode->m_url);
                if (texRes)
                {
                    m_textureSRV = texRes->srv;
                }
            }
        }

        return S_OK;
    }

    void AppearanceImpl::Shutdown()
    {
        m_textureSRV.Release();
        m_material.Release();
    }

    // ShapeImpl
    ShapeImpl::ShapeImpl() {}
    ShapeImpl::~ShapeImpl() { Shutdown(); }

    HRESULT ShapeImpl::Initialize(ShapeNode* node, ID3D11Device* dev, ResourceCache* cache)
    {
        if (!node || !dev) return E_POINTER;

        if (node->m_appearance)
        {
            auto* appNode = dynamic_cast<AppearanceNode*>(node->m_appearance.Get());
            if (appNode)
            {
                m_appearance = new AppearanceImpl();
                m_appearance->Initialize(appNode, dev, cache);
            }
        }

        return S_OK;
    }

    void ShapeImpl::Shutdown()
    {
        m_appearance.Release();
    }

    // LinePropertiesImpl
    HRESULT LinePropertiesImpl::Initialize(LinePropertiesNode* node)
    {
        if (!node) return E_POINTER;
        m_applied = node->m_applied;
        m_lineType = node->m_linetype;
        m_lineWidth = node->m_linewidthScaleFactor;
        return S_OK;
    }

    // FillPropertiesImpl
    HRESULT FillPropertiesImpl::Initialize(FillPropertiesNode* node)
    {
        if (!node) return E_POINTER;
        m_applied = node->m_applied;
        m_filled = node->m_filled;
        return S_OK;
    }

} // namespace HMREngine
