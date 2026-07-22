#pragma once
// TextImpl.h - Text/TextShader/ScrollingText/FontStyle implementations

#include "HMREngine.h"
#include "X3DNodes.h"
#include "Shaders.h"
#include <d3d11.h>
#include <atlbase.h>
#include <string>
#include <vector>

namespace HMREngine
{
    struct FontGlyph
    {
        float u0, v0, u1, v1;
        float advance;
        float width;
        float height;
    };

    class FontAtlas
    {
    public:
        HRESULT Initialize(ID3D11Device* dev, const std::string& fontPath);
        void Shutdown();

        const FontGlyph& GetGlyph(char c) const;
        float GetCharWidth(char c, float fontSize) const;
        float GetTextWidth(const std::string& text, float fontSize) const;
        float GetLineHeight(float fontSize) const;

        ID3D11ShaderResourceView* GetAtlasSRV() const { return m_atlasSRV; }

    private:
        CComPtr<ID3D11Texture2D> m_atlasTexture;
        CComPtr<ID3D11ShaderResourceView> m_atlasSRV;
        std::vector<FontGlyph> m_glyphs;
        float m_atlasWidth = 256.0f;
        float m_atlasHeight = 256.0f;

        void BuildDefaultAtlas();
    };

    struct TextVertex
    {
        Vec3 position;
        Vec2 uv;
    };

    class TextImpl : public ImplObject<TextImpl>
    {
    public:
        TextImpl();
        virtual ~TextImpl();

        HRESULT Initialize(ID3D11Device* dev);
        void Shutdown();

        void SetText(const std::string& text);
        void SetPosition(const Vec3& pos);
        void SetSize(float size);
        void SetColor(const Rgba& color);

        void Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj, float time);

        ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer; }
        UINT GetVertexCount() const { return m_vertexCount; }

    private:
        std::string m_text;
        Vec3 m_position;
        float m_size = 1.0f;
        Rgba m_color = Rgba(1, 1, 1, 1);

        ID3D11Device* m_device = nullptr;
        CComPtr<ID3D11Buffer> m_vertexBuffer;
        UINT m_vertexCount = 0;

        FontAtlas m_fontAtlas;
        bool m_needsRebuild = true;

        void RebuildVertices();
    };

    class ScrollingTextImpl : public ImplObject<ScrollingTextImpl>
    {
    public:
        ScrollingTextImpl();
        virtual ~ScrollingTextImpl();

        HRESULT Initialize(ID3D11Device* dev);
        void Shutdown();

        void SetText(const std::string& text);
        void SetScrollSpeed(float speed);
        void SetPosition(const Vec3& pos);
        void SetSize(float size);
        void SetColor(const Rgba& color);

        void Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj, float time);

    private:
        std::string m_text;
        Vec3 m_position;
        float m_size = 1.0f;
        Rgba m_color = Rgba(1, 1, 1, 1);
        float m_scrollSpeed = 1.0f;

        TextImpl m_textImpl;
        bool m_initialized = false;
    };

    class FontStyleImpl : public ImplObject<FontStyleImpl>
    {
    public:
        FontStyleImpl();
        virtual ~FontStyleImpl();

        HRESULT Initialize(FontStyleNode* node);
        void Shutdown();

        const std::string& GetFamily() const { return m_family; }
        float GetSize() const { return m_size; }
        bool IsBold() const { return m_bold; }
        bool IsItalic() const { return m_italic; }

    private:
        std::string m_family;
        float m_size = 1.0f;
        bool m_bold = false;
        bool m_italic = false;
    };

} // namespace HMREngine
