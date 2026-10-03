---
sidebar_position: 6
title: GPURenderer.dll
description: The D3D11 + D2D1.1 device wrapper for the engine layer — draw video frames via shared textures.
---

# GPURenderer.dll

| | |
|---|---|
| **Source** | `src/Renderer/` |
| **CMake target** | `GPURenderer` (DLL, `OUTPUT_NAME GPURenderer`) |
| **Type** | Win32 DLL |
| **Family** | [DirectUI engine layer](../dui-engine) |

## At a glance

A focused rendering device wrapper in the `DirectUI` namespace: owns a **D3D11 device**
and a **D2D1.1 device context** bound to it, and exposes a minimal draw/present surface —
including `DrawVideoFrame(ID3D11Texture2D*, RECT)` for video compositing. The engine
layer's rendering workhorse (MovieMakerCore instead uses its own
[HMREngine](../../architecture/moviemakercore/hmr-engine.md)).

## Public surface (header `Renderer.h`)

```cpp
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

    ID3D11Device*     GetDevice() const;
    ID2D1DeviceContext* GetD2DContext() const;
    HRESULT DrawVideoFrame(ID3D11Texture2D* pTexture, const RECT& destRect);

private:
    void Cleanup();
    ID3D11Device*       m_pDevice   = nullptr;
    ID2D1DeviceContext* m_pD2DContext = nullptr;
    // ...
};

} // namespace DirectUI
```

## Implementation notes

- **D3D11 → D2D1.1 interop**: the D2D context is created on top of the D3D11 device
  (via DXGI), so D2D drawing and D3D video textures share one swap chain — the same
  interop strategy HMRAVSource's `TextureInterop` uses in the main engine.
- `BeginDraw`/`EndDraw` bracket D2D batch work; `Present` flips the swap chain.
- `Resize` handles swap-chain buffer recreation (window resize).
- Links `d3d11`, `d2d1`, `dxgi` (through the common set) — no D3DX dependency (see
  [SDK Compatibility](../../reference/sdk-compatibility.md)).

## Testing

- Built in the CI matrix as `GPURenderer`.
- Consumed by [PlaybackEngine](./playbackengine.md), which receives a `GPURenderer*` at
  construction.

## Analysis artifacts

Engine-layer reconstruction; for the original engine's renderer analysis see
`analysis/MovieMakerCore/` and `analysis/D3DCOMPILER_46/`.
