#pragma once
// TextResourceDX.h - D3D11 text rendering resources

#include "DXResources.h"

namespace HMREngine
{
    namespace DX
    {
        // --- TextResourceBaseDX: Base text rendering resource ---
        class TextResourceBaseDX : public DeviceClientDXImpl
        {
        public:
            TextResourceBaseDX();
            virtual ~TextResourceBaseDX();

            HRESULT Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx);
            void Release();

            HRESULT BeginTextLayout(UINT width, UINT height);
            HRESULT EndTextLayout();

            IDWriteFactory* GetDWriteFactory() const { return m_dwriteFactory; }
            ID2D1Factory* GetD2DFactory() const { return m_d2dFactory; }
            ID2D1RenderTarget* GetRT() const { return m_renderTarget; }

            UINT GetBitmapWidth() const { return m_bitmapWidth; }
            UINT GetBitmapHeight() const { return m_bitmapHeight; }

        protected:
            CComPtr<IDWriteFactory> m_dwriteFactory;
            CComPtr<ID2D1Factory> m_d2dFactory;
            CComPtr<ID2D1RenderTarget> m_renderTarget;
            CComPtr<ID2D1Bitmap> m_bitmap;
            CComPtr<IWICImagingFactory> m_wicFactory;
            CComPtr<IWICBitmap> m_wicBitmap;

            UINT m_bitmapWidth = 0;
            UINT m_bitmapHeight = 0;

            HRESULT CreateDWriteFactory();
            HRESULT CreateD2DFactory();
            HRESULT CreateWICBitmap(UINT width, UINT height);
        };

        // --- TextResourceDX: D3D11 text texture rendering ---
        class TextResourceDX : public TextResourceBaseDX
        {
        public:
            TextResourceDX();
            virtual ~TextResourceDX();

            HRESULT RenderText(const std::wstring& text,
                const std::wstring& fontFamily = L"Segoe UI",
                float fontSize = 24.0f,
                const Rgba& color = Rgba(1.0f, 1.0f, 1.0f, 1.0f),
                bool bold = false, bool italic = false);

            HRESULT GetTexture(ID3D11Texture2D** ppTex) const;
            HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;

            UINT GetTextWidth() const { return m_textWidth; }
            UINT GetTextHeight() const { return m_textHeight; }

        protected:
            CComPtr<ID3D11Texture2D> m_texture;
            CComPtr<ID3D11ShaderResourceView> m_srv;
            UINT m_textWidth = 0;
            UINT m_textHeight = 0;

            HRESULT CreateD3DTexture(UINT width, UINT height);
            HRESULT UpdateTextureFromBitmap();
        };

        // --- ScrollingTextResourceDX: Scrolling text for credits ---
        class ScrollingTextResourceDX : public TextResourceBaseDX
        {
        public:
            ScrollingTextResourceDX();
            virtual ~ScrollingTextResourceDX();

            HRESULT Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx,
                UINT viewportWidth, UINT viewportHeight);
            void Release();

            HRESULT RenderScrollingText(const std::vector<std::wstring>& lines,
                float scrollOffset,
                const std::wstring& fontFamily = L"Segoe UI",
                float fontSize = 24.0f,
                const Rgba& color = Rgba(1.0f, 1.0f, 1.0f, 1.0f));

            HRESULT GetTexture(ID3D11Texture2D** ppTex) const;
            HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;
            float GetTotalTextHeight() const { return m_totalTextHeight; }

        protected:
            CComPtr<ID3D11Texture2D> m_texture;
            CComPtr<ID3D11ShaderResourceView> m_srv;
            UINT m_viewportWidth = 0;
            UINT m_viewportHeight = 0;
            float m_totalTextHeight = 0.0f;

            HRESULT CreateScrollingResources(UINT width, UINT height);
        };

        // --- PartialTextRendererDX: Partial/incremental text rendering ---
        class PartialTextRendererDX : public TextResourceBaseDX
        {
        public:
            PartialTextRendererDX();
            virtual ~PartialTextRendererDX();

            HRESULT Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx,
                UINT canvasWidth, UINT canvasHeight);
            void Release();

            HRESULT RenderPartialText(const std::wstring& text,
                float x, float y,
                const std::wstring& fontFamily = L"Segoe UI",
                float fontSize = 24.0f,
                const Rgba& color = Rgba(1.0f, 1.0f, 1.0f, 1.0f));

            HRESULT Clear();

            HRESULT GetTexture(ID3D11Texture2D** ppTex) const;
            HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;

        protected:
            CComPtr<ID3D11Texture2D> m_texture;
            CComPtr<ID3D11ShaderResourceView> m_srv;
            UINT m_canvasWidth = 0;
            UINT m_canvasHeight = 0;

            HRESULT CreateCanvasResources(UINT width, UINT height);
        };

    } // namespace DX
} // namespace HMREngine
