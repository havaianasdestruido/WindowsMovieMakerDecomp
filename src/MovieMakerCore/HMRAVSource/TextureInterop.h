/*
 * TextureInterop.h
 *
 * TextureInterop - GPU texture sharing between Media Foundation and Direct3D.
 * Provides a bridge for zero-copy texture transfer between MF video samples
 * and D3D surfaces for rendering and processing.
 *
 * TextureInterOp - Base interface for GPU texture sharing.
 * TextureInterOpDX9 - DX9 texture interop (legacy path).
 * TextureInterOpDX11 - DX11 texture interop (primary path).
 *
 * RTTI: ?AVTextureInterOp@@, ?AVTextureInterOpDX9@@, ?AVTextureInterOpDX11@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef TEXTUREINTEROP_H
#define TEXTUREINTEROP_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// TextureInteropDesc
// ============================================================================
struct TextureInteropDesc
{
    UINT        uWidth;
    UINT        uHeight;
    D3DFORMAT   d3dFormat;
    UINT        uUsage;
    bool        fShared;

    TextureInteropDesc()
        : uWidth(0)
        , uHeight(0)
        , d3dFormat(D3DFMT_A8R8G8B8)
        , uUsage(0)
        , fShared(false)
    {
    }
};

// ============================================================================
// TextureInterOp
// ============================================================================
// Base class for GPU texture sharing between Media Foundation and Direct3D.
// Defines the interface for creating, locking, and transferring textures
// between MF video samples and D3D surfaces.
//
class AVSOURCE_API TextureInterOp
{
public:
    TextureInterOp();
    virtual ~TextureInterOp();

    // Lifecycle
    virtual HRESULT Initialize(const TextureInteropDesc& desc);
    virtual HRESULT Shutdown();

    // Texture operations
    virtual HRESULT CreateTexture(UINT uWidth, UINT uHeight, D3DFORMAT format);
    virtual HRESULT LockRect(UINT uSubresource, void** ppData, UINT* pPitch, DWORD dwFlags);
    virtual HRESULT UnlockRect(UINT uSubresource);
    virtual HRESULT GetSurface(IDirect3DSurface9** ppSurface);

    // Sample interop
    virtual HRESULT CopyFromSample(IMFSample* pSample);
    virtual HRESULT CopyToSample(IMFSample* pSample);

    // Query
    bool IsInitialized() const throw();
    UINT GetWidth() const throw();
    UINT GetHeight() const throw();
    D3DFORMAT GetFormat() const throw();

    // Shared handle
    HRESULT GetSharedHandle(HANDLE* phShared);

protected:
    TextureInteropDesc  m_desc;
    bool                m_fInitialized;
    HANDLE              m_hSharedHandle;

    CComPtr<IDirect3DTexture9>     m_spTexture;
    CComPtr<IDirect3DSurface9>     m_spSurface;

    virtual HRESULT CreateTextureInternal();
    virtual HRESULT CreateSurfaceInternal();
};

// ============================================================================
// TextureInterOpDX9
// ============================================================================
// DX9 texture interop for legacy rendering path. Uses Direct3D 9 surfaces
// and textures for MF sample transfer. Provides the fallback path when
// DX11 is not available or for legacy content.
//
class AVSOURCE_API TextureInterOpDX9 : public TextureInterOp
{
public:
    TextureInterOpDX9();
    virtual ~TextureInterOpDX9();

    HRESULT Initialize(const TextureInteropDesc& desc, IDirect3DDevice9* pDevice);
    HRESULT Shutdown() override;

    // DX9-specific operations
    HRESULT CreateTextureFromDevice(IDirect3DDevice9* pDevice);
    HRESULT CopyFromSurface(IDirect3DSurface9* pSource);
    HRESULT CopyToSurface(IDirect3DSurface9* pDest);

    // Device access
    IDirect3DDevice9* GetDevice() const;
    HRESULT SetDevice(IDirect3DDevice9* pDevice);

    // Shared texture support
    HRESULT CreateSharedTexture(IDirect3DDevice9* pDevice, HANDLE* phShared);
    HRESULT OpenSharedTexture(IDirect3DDevice9* pDevice, HANDLE hShared);

private:
    IDirect3DDevice9*   m_pDevice;
    HANDLE              m_hSharedTexture;

    HRESULT CreateTextureInternal() override;
    HRESULT CreateSurfaceInternal() override;
};

// ============================================================================
// TextureInterOpDX11
// ============================================================================
// DX11 texture interop for the primary rendering path. Uses Direct3D 11
// textures with shared handles for zero-copy transfer between MF and D3D11.
// Provides hardware-accelerated video rendering.
//
class AVSOURCE_API TextureInterOpDX11 : public TextureInterOp
{
public:
    TextureInterOpDX11();
    virtual ~TextureInterOpDX11();

    HRESULT Initialize(const TextureInteropDesc& desc, ID3D11Device* pDevice);
    HRESULT Shutdown() override;

    // DX11-specific operations
    HRESULT CreateTextureFromDevice(ID3D11Device* pDevice);
    HRESULT CopyFromTexture(ID3D11Texture2D* pSource);
    HRESULT CopyToTexture(ID3D11Texture2D* pDest);

    // Shared handle operations
    HRESULT CreateSharedTexture(ID3D11Device* pDevice, HANDLE* phShared);
    HRESULT OpenSharedTexture(ID3D11Device* pDevice, HANDLE hShared);

    // Device access
    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetContext() const;
    HRESULT SetDevice(ID3D11Device* pDevice);

    // GPU/CPU access
    HRESULT Map(D3D11_MAP mapType, void** ppData, UINT* pRowPitch);
    HRESULT Unmap();

    // Query
    D3D11_TEXTURE2D_DESC GetTextureDesc() const;

private:
    ID3D11Device*           m_pDevice;
    ID3D11DeviceContext*    m_pContext;
    CComPtr<ID3D11Texture2D> m_spDX11Texture;
    HANDLE                  m_hSharedTexture;

    HRESULT CreateTextureInternal() override;
    HRESULT CreateSurfaceInternal() override;
    HRESULT CreateDX11Texture(ID3D11Device* pDevice);
};

} // namespace HMRAVSource

#endif // TEXTUREINTEROP_H
