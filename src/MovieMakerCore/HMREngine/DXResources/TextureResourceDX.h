#pragma once
// TextureResourceDX.h - D3D11 texture resource wrappers

#include "DXResources.h"
#include "DeviceClientDX.h"

namespace HMREngine
{
    namespace DX
    {
        // --- TextureResourceDX: D3D11 texture wrapper ---
        class TextureResourceDX : public DeviceClientDXImpl
        {
        public:
            TextureResourceDX();
            virtual ~TextureResourceDX();

            HRESULT LoadFromFile(const std::wstring& filePath);
            HRESULT LoadFromFile(const std::string& filePath);
            HRESULT CreateFromData(const void* data, UINT width, UINT height,
                DXGI_FORMAT format, UINT rowPitch = 0);
            HRESULT CreateEmpty(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
            HRESULT CreateFromDIB(const void* dibData, UINT dataSize);
            void Release();

            HRESULT GetTexture(ID3D11Texture2D** ppTex) const;
            HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;

            UINT GetWidth() const { return m_width; }
            UINT GetHeight() const { return m_height; }
            DXGI_FORMAT GetFormat() const { return m_format; }
            bool IsLoaded() const { return m_loaded; }
            const std::wstring& GetFilePath() const { return m_filePath; }

        protected:
            CComPtr<ID3D11Texture2D> m_texture;
            CComPtr<ID3D11ShaderResourceView> m_srv;
            std::wstring m_filePath;
            UINT m_width = 0;
            UINT m_height = 0;
            DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
            bool m_loaded = false;

            HRESULT CreateTextureFromWIC(const void* data, UINT size);
            HRESULT CreateSRV();
        };

        // --- TextureResourceFromResourceDX: Load from embedded .NET resource ---
        class TextureResourceFromResourceDX : public TextureResourceDX
        {
        public:
            TextureResourceFromResourceDX();
            virtual ~TextureResourceFromResourceDX();

            HRESULT LoadFromResource(HMODULE hModule, UINT resourceId, const std::wstring& resourceType = L"IMAGE");
            HRESULT LoadFromResource(HMODULE hModule, const std::wstring& resourceName, const std::wstring& resourceType);

        protected:
            HMODULE m_resourceModule = nullptr;
            UINT m_resourceId = 0;

            HRESULT ExtractResourceData(HMODULE hModule, UINT resourceId,
                const std::wstring& resourceType, std::vector<BYTE>& outData);
        };

        // --- MotionTextureResourceDX: Animated texture for motion blur ---
        class MotionTextureResourceDX : public DeviceClientDXImpl
        {
        public:
            MotionTextureResourceDX();
            virtual ~MotionTextureResourceDX();

            HRESULT LoadFromFiles(const std::vector<std::wstring>& framePaths);
            HRESULT LoadFromStrip(const std::wstring& stripPath, UINT frameCount,
                UINT frameWidth, UINT frameHeight);
            void Release();

            HRESULT GetTextureForFrame(UINT frameIndex, ID3D11ShaderResourceView** ppSRV);
            HRESULT GetTextureAtTime(double time, UINT fps, ID3D11ShaderResourceView** ppSRV);

            UINT GetFrameCount() const { return m_frameCount; }
            UINT GetFrameWidth() const { return m_frameWidth; }
            UINT GetFrameHeight() const { return m_frameHeight; }
            void SetOffset(float offset) { m_offset = offset; }
            float GetOffset() const { return m_offset; }
            void SetScale(float scale) { m_scale = scale; }
            float GetScale() const { return m_scale; }

        protected:
            std::vector<CComPtr<ID3D11Texture2D>> m_frames;
            std::vector<CComPtr<ID3D11ShaderResourceView>> m_srvs;
            CComPtr<ID3D11Texture2D> m_stripTexture;
            UINT m_frameCount = 0;
            UINT m_frameWidth = 0;
            UINT m_frameHeight = 0;
            float m_offset = 0.0f;
            float m_scale = 1.0f;
            bool m_isStrip = false;

            HRESULT CreateFrameFromStripData(const BYTE* stripData, UINT stripSize,
                UINT frameIndex, ID3D11Texture2D** ppTex);
        };

    } // namespace DX
} // namespace HMREngine
