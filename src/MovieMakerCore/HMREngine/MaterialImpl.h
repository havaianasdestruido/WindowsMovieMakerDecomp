#pragma once
// MaterialImpl.h - Material, Appearance, Shape, LineProperties, FillProperties

#include "HMREngine.h"
#include "X3DNodes.h"
#include <d3d11.h>
#include <atlbase.h>

namespace HMREngine
{
    struct MaterialBuffer
    {
        Rgb diffuseColor;
        float pad0;
        Rgb specularColor;
        float shininess;
        Rgb emissiveColor;
        float transparency;
        Rgb ambientColor;
        float pad1;
    };

    class MaterialImpl : public ImplObject<MaterialImpl>
    {
    public:
        MaterialImpl();
        virtual ~MaterialImpl();

        HRESULT Initialize(MaterialNode* node, ID3D11Device* dev);
        void Shutdown();

        void UpdateBuffer(ID3D11DeviceContext* ctx);
        ID3D11Buffer* GetConstantBuffer() const { return m_constantBuffer; }
        const MaterialBuffer& GetBufferData() const { return m_bufferData; }

        Rgb GetDiffuseColor() const { return m_diffuse; }
        Rgb GetSpecularColor() const { return m_specular; }
        Rgb GetEmissiveColor() const { return m_emissive; }
        float GetShininess() const { return m_shininess; }
        float GetTransparency() const { return m_transparency; }
        bool IsTranslucent() const { return m_transparency > 0.0f; }

    private:
        MaterialBuffer m_bufferData{};
        CComPtr<ID3D11Buffer> m_constantBuffer;

        Rgb m_diffuse;
        Rgb m_specular;
        Rgb m_emissive;
        Rgb m_ambient;
        float m_shininess = 0.2f;
        float m_transparency = 0.0f;
    };

    class AppearanceImpl : public ImplObject<AppearanceImpl>
    {
    public:
        AppearanceImpl();
        virtual ~AppearanceImpl();

        HRESULT Initialize(AppearanceNode* node, ID3D11Device* dev, ResourceCache* cache);
        void Shutdown();

        MaterialImpl* GetMaterial() const { return m_material; }
        ID3D11ShaderResourceView* GetTexture() const { return m_textureSRV; }

    private:
        SAIPtr<MaterialImpl> m_material;
        CComPtr<ID3D11ShaderResourceView> m_textureSRV;
    };

    class ShapeImpl : public ImplObject<ShapeImpl>
    {
    public:
        ShapeImpl();
        virtual ~ShapeImpl();

        HRESULT Initialize(ShapeNode* node, ID3D11Device* dev, ResourceCache* cache);
        void Shutdown();

        AppearanceImpl* GetAppearance() const { return m_appearance; }
        Matrix4f GetWorldMatrix() const { return m_worldMatrix; }
        void SetWorldMatrix(const Matrix4f& m) { m_worldMatrix = m; }

    private:
        SAIPtr<AppearanceImpl> m_appearance;
        Matrix4f m_worldMatrix;
    };

    class LinePropertiesImpl : public ImplObject<LinePropertiesImpl>
    {
    public:
        HRESULT Initialize(LinePropertiesNode* node);
        bool IsApplied() const { return m_applied; }
        int GetLineType() const { return m_lineType; }
        float GetLineWidth() const { return m_lineWidth; }
    private:
        bool m_applied = true;
        int m_lineType = 1;
        float m_lineWidth = 1.0f;
    };

    class FillPropertiesImpl : public ImplObject<FillPropertiesImpl>
    {
    public:
        HRESULT Initialize(FillPropertiesNode* node);
        bool IsApplied() const { return m_applied; }
        bool IsFilled() const { return m_filled; }
    private:
        bool m_applied = true;
        bool m_filled = true;
    };

} // namespace HMREngine
