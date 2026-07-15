#include "pch.h"

// HMREngine.cpp - Engine lifecycle, render loop, D3D11 device management

#include "HMREngine.h"

namespace HMREngine
{
    // Global initialization
    static bool g_initialized = false;

    HRESULT InitializeHMREngine()
    {
        if (g_initialized) return S_OK;
        g_initialized = true;
        return S_OK;
    }

    void ShutdownHMREngine()
    {
        g_initialized = false;
    }

    bool IsHMREngineInitialized()
    {
        return g_initialized;
    }

} // namespace HMREngine
