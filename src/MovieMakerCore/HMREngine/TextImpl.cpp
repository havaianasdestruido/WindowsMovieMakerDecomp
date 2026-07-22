#include "pch.h"

// TextImpl.cpp - Text/TextShader/ScrollingText/FontStyle implementations

#include "TextImpl.h"

namespace HMREngine
{
    // FontAtlas
    void FontAtlas::BuildDefaultAtlas()
    {
        m_glyphs.resize(128);
        float cellW = m_atlasWidth / 16.0f;
        float cellH = m_atlasHeight / 8.0f;

        for (int i = 0; i < 128; i++)
        {
            int col = i % 16;
            int row = i / 16;
            auto& g = m_glyphs[i];
            g.u0 = col * cellW / m_atlasWidth;
            g.v0 = row * cellH / m_atlasHeight;
            g.u1 = (col + 1) * cellW / m_atlasWidth;
            g.v1 = (row + 1) * cellH / m_atlasHeight;
            g.advance = cellW * 0.6f;
            g.width = cellW * 0.6f;
            g.height = cellH;
        }
    }

    HRESULT FontAtlas::Initialize(ID3D11Device* dev, const std::string& fontPath)
    {
        BuildDefaultAtlas();

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(m_atlasWidth);
        td.Height = static_cast<UINT>(m_atlasHeight);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        // Generate simple bitmap font
        std::vector<UINT32> pixels(static_cast<size_t>(m_atlasWidth * m_atlasHeight), 0x00000000);
        float cellW = m_atlasWidth / 16.0f;
        float cellH = m_atlasHeight / 8.0f;

        for (int i = 32; i < 127; i++)
        {
            int col = i % 16;
            int row = i / 16;
            int ox = static_cast<int>(col * cellW);
            int oy = static_cast<int>(row * cellH);
            int w = static_cast<int>(cellW * 0.6f);
            int h = static_cast<int>(cellH * 0.8f);

            for (int y = 0; y < h && (oy + y) < static_cast<int>(m_atlasHeight); y++)
            {
                for (int x = 0; x < w && (ox + x) < static_cast<int>(m_atlasWidth); x++)
                {
                    // Simple block character
                    if (x == 0 || x == w - 1 || y == 0 || y == h - 1 ||
                        (i >= 'A' && i <= 'Z'))
                    {
                        size_t idx = (oy + y) * static_cast<size_t>(m_atlasWidth) + (ox + x);
                        if (idx < pixels.size()) pixels[idx] = 0xFFFFFFFF;
                    }
                }
            }
        }

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = pixels.data();
        initData.SysMemPitch = static_cast<UINT>(m_atlasWidth * 4);

        HRESULT hr = dev->CreateTexture2D(&td, &initData, &m_atlasTexture);
        if (FAILED(hr)) return hr;

        hr = dev->CreateShaderResourceView(m_atlasTexture, nullptr, &m_atlasSRV);
        return hr;
    }

    void FontAtlas::Shutdown()
    {
        m_atlasSRV.Release();
        m_atlasTexture.Release();
    }

    const FontGlyph& FontAtlas::GetGlyph(char c) const
    {
        return m_glyphs[static_cast<int>(c) & 127];
    }

    float FontAtlas::GetCharWidth(char c, float fontSize) const
    {
        return GetGlyph(c).width * fontSize / m_atlasWidth;
    }

    float FontAtlas::GetTextWidth(const std::string& text, float fontSize) const
    {
        float total = 0;
        for (char c : text) total += GetCharWidth(c, fontSize);
        return total;
    }

    float FontAtlas::GetLineHeight(float fontSize) const
    {
        return fontSize;
    }

    // TextImpl
    TextImpl::TextImpl() {}
    TextImpl::~TextImpl() { Shutdown(); }

    HRESULT TextImpl::Initialize(ID3D11Device* dev)
    {
        if (!dev) return E_POINTER;
        m_device = dev;
        return m_fontAtlas.Initialize(dev, "");
    }

    void TextImpl::Shutdown()
    {
        m_vertexBuffer.Release();
        m_fontAtlas.Shutdown();
    }

    void TextImpl::SetText(const std::string& text) { m_text = text; m_needsRebuild = true; }
    void TextImpl::SetPosition(const Vec3& pos) { m_position = pos; m_needsRebuild = true; }
    void TextImpl::SetSize(float size) { m_size = size; m_needsRebuild = true; }
    void TextImpl::SetColor(const Rgba& color) { m_color = color; m_needsRebuild = true; }

    void TextImpl::RebuildVertices()
    {
        if (!m_device) return;

        std::vector<TextVertex> verts;
        float x = 0;
        float y = 0;

        for (char c : m_text)
        {
            if (c == '\n') { x = 0; y += m_fontAtlas.GetLineHeight(m_size); continue; }

            const auto& glyph = m_fontAtlas.GetGlyph(c);
            float gw = glyph.width * m_size / 256.0f;
            float gh = glyph.height * m_size / 256.0f;

            Vec3 p0(m_position.x + x, m_position.y - y, m_position.z);
            Vec3 p1(p0.x + gw, p0.y - gh, p0.z);

            verts.push_back({ p0, Vec2(glyph.u0, glyph.v0) });
            verts.push_back({ p1, Vec2(glyph.u1, glyph.v0) });
            verts.push_back({ p0 + Vec3(gw, 0, 0), Vec2(glyph.u1, glyph.v1) });
            verts.push_back({ p0, Vec2(glyph.u0, glyph.v0) });
            verts.push_back({ p0 + Vec3(0, -gh, 0), Vec2(glyph.u0, glyph.v1) });
            verts.push_back({ p1, Vec2(glyph.u1, glyph.v1) });

            x += glyph.advance * m_size / 256.0f;
        }

        m_vertexCount = static_cast<UINT>(verts.size());

        m_vertexBuffer.Release();

        if (!verts.empty())
        {
            D3D11_BUFFER_DESC bd{};
            bd.ByteWidth = static_cast<UINT>(verts.size() * sizeof(TextVertex));
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData{};
            initData.pSysMem = verts.data();

            m_device->CreateBuffer(&bd, &initData, &m_vertexBuffer);
        }

        m_needsRebuild = false;
    }

    void TextImpl::Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj, float time)
    {
        if (m_text.empty() || !ctx) return;

        if (m_needsRebuild)
            RebuildVertices();

        if (!m_vertexBuffer || m_vertexCount == 0) return;

        UINT stride = sizeof(TextVertex);
        UINT offset = 0;
        ID3D11Buffer* vb = m_vertexBuffer;
        ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->Draw(m_vertexCount, 0);
    }

    // ScrollingTextImpl
    ScrollingTextImpl::ScrollingTextImpl() {}
    ScrollingTextImpl::~ScrollingTextImpl() { Shutdown(); }

    HRESULT ScrollingTextImpl::Initialize(ID3D11Device* dev)
    {
        if (!m_initialized)
        {
            HRESULT hr = m_textImpl.Initialize(dev);
            if (FAILED(hr)) return hr;
            m_initialized = true;
        }
        return S_OK;
    }

    void ScrollingTextImpl::Shutdown()
    {
        m_textImpl.Shutdown();
        m_initialized = false;
    }

    void ScrollingTextImpl::SetText(const std::string& text) { m_text = text; m_textImpl.SetText(text); }
    void ScrollingTextImpl::SetScrollSpeed(float speed) { m_scrollSpeed = speed; }
    void ScrollingTextImpl::SetPosition(const Vec3& pos) { m_position = pos; m_textImpl.SetPosition(pos); }
    void ScrollingTextImpl::SetSize(float size) { m_size = size; m_textImpl.SetSize(size); }
    void ScrollingTextImpl::SetColor(const Rgba& color) { m_color = color; m_textImpl.SetColor(color); }

    void ScrollingTextImpl::Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj, float time)
    {
        Vec3 pos = m_position;
        pos.x -= time * m_scrollSpeed;
        m_textImpl.SetPosition(pos);
        m_textImpl.Render(ctx, worldViewProj, time);
    }

    // FontStyleImpl
    FontStyleImpl::FontStyleImpl() {}
    FontStyleImpl::~FontStyleImpl() { Shutdown(); }

    HRESULT FontStyleImpl::Initialize(FontStyleNode* node)
    {
        if (!node) return E_POINTER;
        m_family = node->m_family;
        m_size = node->m_size;
        m_bold = node->m_bold;
        m_italic = node->m_italic;
        return S_OK;
    }

    void FontStyleImpl::Shutdown()
    {
    }

} // namespace HMREngine
