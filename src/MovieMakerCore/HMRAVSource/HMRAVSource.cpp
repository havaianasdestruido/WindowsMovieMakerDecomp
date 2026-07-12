// HMRAVSource.cpp - Subsystem lifecycle for audio/video source management
//
// Initializes/shuts down COM and Media Foundation for the AV pipeline.

#include "pch.h"
#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// Global state
// ============================================================================
static bool g_fInitialized = false;
static LONG g_cLockCount = 0;

// ============================================================================
// HMRAVSourceInitialize
// ============================================================================
HRESULT HMRAVSourceInitialize()
{
    if (g_fInitialized)
        return S_OK;

    HRESULT hr = S_OK;

    // Initialize COM apartment (MTA for cross-thread MF access)
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return hr;

    // Initialize Media Foundation platform
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
    {
        CoUninitialize();
        return hr;
    }

    g_fInitialized = true;
    return S_OK;
}

// ============================================================================
// HMRAVSourceShutdown
// ============================================================================
void HMRAVSourceShutdown()
{
    if (!g_fInitialized)
        return;

    MFShutdown();
    CoUninitialize();

    g_fInitialized = false;
}

// ============================================================================
// HMRAVSourceIsInitialized
// ============================================================================
bool HMRAVSourceIsInitialized()
{
    return g_fInitialized;
}

// ============================================================================
// Lock / Unlock (reference counting for shared usage)
// ============================================================================
static void AVSourceAddRef()
{
    InterlockedIncrement(&g_cLockCount);
}

static void AVSourceRelease()
{
    if (InterlockedDecrement(&g_cLockCount) == 0)
    {
        // Last reference released - could trigger cleanup
    }
}

} // namespace HMRAVSource
