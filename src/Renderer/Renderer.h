#pragma once
#include <d3d11.h>
#include <d2d1_1.h>

namespace DirectUI {

class __declspec(dllexport) GPURenderer {
public:
    GPURenderer();
    ~GPURenderer();

    HRESULT Initialize(HWND hwnd);
    HRESULT Resize(int width, int height);
    HRESULT BeginDraw();
    HRESULT EndDraw();
    HRESULT Present();

    ID3D11Device* GetDevice() const { return m_pDevice; }
    ID2D1DeviceContext* GetD2DContext() const { return m_pD2DContext; }
    HRESULT DrawVideoFrame(ID3D11Texture2D* pTexture, const RECT& destRect);

private:
    void Cleanup();

    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;
    IDXGISwapChain* m_pSwapChain = nullptr;
    ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

    ID2D1Factory1* m_pD2DFactory = nullptr;
    ID2D1Device* m_pD2DDevice = nullptr;
    ID2D1DeviceContext* m_pD2DContext = nullptr;
    ID2D1Bitmap1* m_pD2DTargetBitmap = nullptr;
};

} // namespace DirectUI
