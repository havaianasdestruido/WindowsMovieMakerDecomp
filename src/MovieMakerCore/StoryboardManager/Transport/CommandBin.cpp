#include "pch.h"
/*
 * CommandBin.cpp
 *
 * Implementation of CommandBin, RenderCommandBin, and DynamicRouteManager.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "CommandBin.h"

// ============================================================================
// CommandBin implementation
// ============================================================================

CommandBin::CommandBin()
    : m_bCoalesceEnabled(true)
    , m_bDirty(false)
    , m_dwPriority(0)
    , m_hrLastError(S_OK)
{
    InitializeCriticalSection(&m_csLock);
}

CommandBin::~CommandBin()
{
    Clear();
    DeleteCriticalSection(&m_csLock);
}

void CommandBin::AddCommand(const CommandEntry& entry)
{
    EnterCriticalSection(&m_csLock);

    bool bHandled = false;
    if (m_bCoalesceEnabled)
    {
        for (auto& existing : m_commands)
        {
            if (ShouldCoalesce(existing, entry))
            {
                try
                {
                    existing = entry;
                    m_bDirty = true;
                }
                catch (const std::bad_alloc&)
                {
                    m_hrLastError = E_OUTOFMEMORY;
                }
                bHandled = true;
                break;
            }
        }
    }

    if (!bHandled)
    {
        try
        {
            m_commands.push_back(entry);
            m_bDirty = true;
        }
        catch (const std::bad_alloc&)
        {
            m_hrLastError = E_OUTOFMEMORY;
        }
    }

    LeaveCriticalSection(&m_csLock);
}

void CommandBin::AddCommand(CommandType cmdType, DWORD dwTargetId, DWORD dwFlags)
{
    CommandEntry entry;
    entry.cmdType = cmdType;
    entry.dwTargetId = dwTargetId;
    entry.dwFlags = dwFlags;
    AddCommand(entry);
}

void CommandBin::AddCommand(CommandType cmdType, DWORD dwTargetId, LONGLONG llArgument)
{
    CommandEntry entry;
    entry.cmdType = cmdType;
    entry.dwTargetId = dwTargetId;
    entry.llArgument = llArgument;
    AddCommand(entry);
}

void CommandBin::AddCommand(CommandType cmdType, DWORD dwTargetId, float flArgument)
{
    CommandEntry entry;
    entry.cmdType = cmdType;
    entry.dwTargetId = dwTargetId;
    entry.flArgument = flArgument;
    AddCommand(entry);
}

void CommandBin::AddCommand(CommandType cmdType, DWORD dwTargetId, LPCWSTR pszArgument)
{
    CommandEntry entry;
    entry.cmdType = cmdType;
    entry.dwTargetId = dwTargetId;
    try
    {
        entry.strArgument = pszArgument ? pszArgument : L"";
    }
    catch (const std::bad_alloc&)
    {
        m_hrLastError = E_OUTOFMEMORY;
        return;
    }
    AddCommand(entry);
}

HRESULT CommandBin::Flush()
{
    EnterCriticalSection(&m_csLock);

    if (!m_bDirty)
    {
        LeaveCriticalSection(&m_csLock);
        return S_FALSE;
    }

    HRESULT hrOverall = S_OK;

    for (auto& cmd : m_commands)
    {
        HRESULT hr = S_OK;

        switch (cmd.cmdType)
        {
        case CommandTypeAddNode:
            if (cmd.dwTargetId == 0 && cmd.strArgument.IsEmpty())
                hr = E_INVALIDARG;
            break;
        case CommandTypeRemoveNode:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            break;
        case CommandTypeSetProperty:
            if (cmd.dwTargetId == 0 || cmd.strArgument.IsEmpty())
                hr = E_INVALIDARG;
            break;
        case CommandTypeConnectRoute:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            break;
        case CommandTypeDisconnectRoute:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            break;
        case CommandTypeSetTransform:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            break;
        case CommandTypeSetVisibility:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            break;
        case CommandTypeSetOpacity:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            else if (cmd.flArgument < 0.0f || cmd.flArgument > 1.0f)
                hr = E_INVALIDARG;
            break;
        case CommandTypePlayAnimation:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            break;
        case CommandTypeStopAnimation:
            if (cmd.dwTargetId == 0)
                hr = E_INVALIDARG;
            break;
        case CommandTypeSetTimelineClock:
            if (cmd.llArgument < 0)
                hr = E_INVALIDARG;
            break;
        default:
            hr = E_NOTIMPL;
            break;
        }

        if (FAILED(hr))
            hrOverall = hr;
    }

    // Transactional commit/rollback: only clear the buffer when every
    // command validated. On failure keep the batch intact so the caller
    // can fix or discard it, and mark the bin dirty for a retry.
    if (SUCCEEDED(hrOverall))
    {
        m_commands.clear();
        m_bDirty = false;
        m_hrLastError = S_OK;
    }

    LeaveCriticalSection(&m_csLock);
    return hrOverall;
}

void CommandBin::Clear()
{
    EnterCriticalSection(&m_csLock);
    m_commands.clear();
    m_bDirty = false;
    m_hrLastError = S_OK;
    LeaveCriticalSection(&m_csLock);
}

size_t CommandBin::GetCommandCount() const throw()
{
    EnterCriticalSection(&m_csLock);
    size_t cCount = m_commands.size();
    LeaveCriticalSection(&m_csLock);
    return cCount;
}

bool CommandBin::IsDirty() const throw()
{
    EnterCriticalSection(&m_csLock);
    bool bDirty = m_bDirty;
    LeaveCriticalSection(&m_csLock);
    return bDirty;
}

void CommandBin::EnableCoalescing(bool bEnable)
{
    EnterCriticalSection(&m_csLock);
    m_bCoalesceEnabled = bEnable;
    LeaveCriticalSection(&m_csLock);
}

bool CommandBin::IsCoalescingEnabled() const throw()
{
    EnterCriticalSection(&m_csLock);
    bool bEnabled = m_bCoalesceEnabled;
    LeaveCriticalSection(&m_csLock);
    return bEnabled;
}

void CommandBin::SetPriority(DWORD dwPriority)
{
    EnterCriticalSection(&m_csLock);
    m_dwPriority = dwPriority;
    LeaveCriticalSection(&m_csLock);
}

DWORD CommandBin::GetPriority() const throw()
{
    EnterCriticalSection(&m_csLock);
    DWORD dwPriority = m_dwPriority;
    LeaveCriticalSection(&m_csLock);
    return dwPriority;
}

HRESULT CommandBin::GetLastError() const throw()
{
    EnterCriticalSection(&m_csLock);
    HRESULT hr = m_hrLastError;
    LeaveCriticalSection(&m_csLock);
    return hr;
}

bool CommandBin::ShouldCoalesce(const CommandEntry& existing, const CommandEntry& incoming)
{
    // Coalesce commands that target the same node with the same type
    return (existing.cmdType == incoming.cmdType &&
            existing.dwTargetId == incoming.dwTargetId);
}

// ============================================================================
// RenderCommandBin implementation
// ============================================================================

RenderCommandBin::RenderCommandBin()
    : CommandBin()
    , m_dwRenderTargetId(0)
    , m_hrLastRenderResult(S_OK)
    , m_flClearColorR(0.0f)
    , m_flClearColorG(0.0f)
    , m_flClearColorB(0.0f)
    , m_flClearColorA(1.0f)
{
    SetRect(&m_rcViewport, 0, 0, 1920, 1080);
}

RenderCommandBin::~RenderCommandBin()
{
}

void RenderCommandBin::SetRenderTarget(DWORD dwTargetId)
{
    EnterCriticalSection(&m_csLock);
    m_dwRenderTargetId = dwTargetId;
    m_bDirty = true;
    LeaveCriticalSection(&m_csLock);
}

void RenderCommandBin::SetViewport(const RECT& rcViewport)
{
    EnterCriticalSection(&m_csLock);
    m_rcViewport = rcViewport;
    m_bDirty = true;
    LeaveCriticalSection(&m_csLock);
}

void RenderCommandBin::SetShaderParameter(DWORD dwParamId, const float* pValues, DWORD dwCount)
{
    if (!pValues || dwCount == 0)
        return;

    ShaderParamEntry entry;
    entry.dwParamId = dwParamId;

    EnterCriticalSection(&m_csLock);
    try
    {
        entry.values.assign(pValues, pValues + dwCount);
        m_shaderParams.push_back(std::move(entry));
        m_bDirty = true;
    }
    catch (const std::bad_alloc&)
    {
        m_hrLastError = E_OUTOFMEMORY;
    }
    LeaveCriticalSection(&m_csLock);
}

void RenderCommandBin::BindTexture(DWORD dwTextureId, DWORD dwSlot)
{
    TextureBinding binding;
    binding.dwTextureId = dwTextureId;
    binding.dwSlot = dwSlot;

    EnterCriticalSection(&m_csLock);
    try
    {
        m_textureBindings.push_back(binding);
        m_bDirty = true;
    }
    catch (const std::bad_alloc&)
    {
        m_hrLastError = E_OUTOFMEMORY;
    }
    LeaveCriticalSection(&m_csLock);
}

void RenderCommandBin::SetClearColor(float flR, float flG, float flB, float flA)
{
    EnterCriticalSection(&m_csLock);
    m_flClearColorR = flR;
    m_flClearColorG = flG;
    m_flClearColorB = flB;
    m_flClearColorA = flA;
    m_bDirty = true;
    LeaveCriticalSection(&m_csLock);
}

HRESULT RenderCommandBin::Flush()
{
    HRESULT hr = CommandBin::Flush();
    EnterCriticalSection(&m_csLock);
    m_hrLastRenderResult = hr;
    LeaveCriticalSection(&m_csLock);
    return hr;
}

HRESULT RenderCommandBin::GetLastRenderResult() const throw()
{
    EnterCriticalSection(&m_csLock);
    HRESULT hr = m_hrLastRenderResult;
    LeaveCriticalSection(&m_csLock);
    return hr;
}

// ============================================================================
// DynamicRouteManager implementation
// ============================================================================

DynamicRouteManager::DynamicRouteManager()
    : m_dwNextRouteId(1)
{
}

DynamicRouteManager::~DynamicRouteManager()
{
    DisconnectAllRoutes();
    m_routes.clear();
}

HRESULT DynamicRouteManager::AddRoute(
    DWORD dwSourceNodeId, LPCWSTR pszSourceField,
    DWORD dwTargetNodeId, LPCWSTR pszTargetField)
{
    if (!pszSourceField || !pszTargetField)
        return E_POINTER;

    RouteEntry entry;
    entry.dwSourceNodeId = dwSourceNodeId;
    entry.dwTargetNodeId = dwTargetNodeId;
    entry.bConnected = false;

    try
    {
        entry.strSourceField = pszSourceField;
        entry.strTargetField = pszTargetField;
        m_routes.push_back(entry);
    }
    catch (const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }

    // Assign the route ID only after a successful insert so a failed
    // insert does not consume (and lose) a route ID.
    m_routes.back().dwRouteId = m_dwNextRouteId++;
    return S_OK;
}

HRESULT DynamicRouteManager::RemoveRoute(DWORD dwRouteId)
{
    for (auto it = m_routes.begin(); it != m_routes.end(); ++it)
    {
        if (it->dwRouteId == dwRouteId)
        {
            if (it->bConnected)
                DisconnectRoute(dwRouteId);
            m_routes.erase(it);
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT DynamicRouteManager::ConnectRoute(DWORD dwRouteId)
{
    for (auto& route : m_routes)
    {
        if (route.dwRouteId == dwRouteId)
        {
            if (route.bConnected)
                return S_FALSE;

            if (route.dwSourceNodeId == 0 || route.dwTargetNodeId == 0)
                return E_INVALIDARG;

            if (route.strSourceField.IsEmpty() || route.strTargetField.IsEmpty())
                return E_INVALIDARG;

            route.bConnected = true;
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT DynamicRouteManager::DisconnectRoute(DWORD dwRouteId)
{
    for (auto& route : m_routes)
    {
        if (route.dwRouteId == dwRouteId)
        {
            if (!route.bConnected)
                return S_FALSE;

            route.bConnected = false;
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT DynamicRouteManager::DisconnectAllRoutes()
{
    HRESULT hrOverall = S_OK;
    for (auto& route : m_routes)
    {
        if (route.bConnected)
        {
            HRESULT hr = DisconnectRoute(route.dwRouteId);
            if (FAILED(hr))
                hrOverall = hr;
        }
    }
    return hrOverall;
}

HRESULT DynamicRouteManager::ConnectAllRoutes()
{
    HRESULT hrOverall = S_OK;
    for (auto& route : m_routes)
    {
        if (!route.bConnected)
        {
            HRESULT hr = ConnectRoute(route.dwRouteId);
            if (FAILED(hr))
                hrOverall = hr;
        }
    }
    return hrOverall;
}

size_t DynamicRouteManager::GetRouteCount() const throw()
{
    return m_routes.size();
}

const DynamicRouteManager::RouteEntry* DynamicRouteManager::GetRoute(DWORD dwRouteId) const
{
    for (const auto& route : m_routes)
    {
        if (route.dwRouteId == dwRouteId)
            return &route;
    }
    return nullptr;
}

const DynamicRouteManager::RouteEntry* DynamicRouteManager::GetRouteAt(size_t nIndex) const
{
    if (nIndex < m_routes.size())
        return &m_routes[nIndex];
    return nullptr;
}

HRESULT DynamicRouteManager::ConnectRoutesBySource(DWORD dwSourceNodeId)
{
    HRESULT hrOverall = S_OK;
    for (auto& route : m_routes)
    {
        if (route.dwSourceNodeId == dwSourceNodeId && !route.bConnected)
        {
            HRESULT hr = ConnectRoute(route.dwRouteId);
            if (FAILED(hr))
                hrOverall = hr;
        }
    }
    return hrOverall;
}

HRESULT DynamicRouteManager::ConnectRoutesByTarget(DWORD dwTargetNodeId)
{
    HRESULT hrOverall = S_OK;
    for (auto& route : m_routes)
    {
        if (route.dwTargetNodeId == dwTargetNodeId && !route.bConnected)
        {
            HRESULT hr = ConnectRoute(route.dwRouteId);
            if (FAILED(hr))
                hrOverall = hr;
        }
    }
    return hrOverall;
}

HRESULT DynamicRouteManager::DisconnectRoutesBySource(DWORD dwSourceNodeId)
{
    HRESULT hrOverall = S_OK;
    for (auto& route : m_routes)
    {
        if (route.dwSourceNodeId == dwSourceNodeId && route.bConnected)
        {
            HRESULT hr = DisconnectRoute(route.dwRouteId);
            if (FAILED(hr))
                hrOverall = hr;
        }
    }
    return hrOverall;
}

HRESULT DynamicRouteManager::DisconnectRoutesByTarget(DWORD dwTargetNodeId)
{
    HRESULT hrOverall = S_OK;
    for (auto& route : m_routes)
    {
        if (route.dwTargetNodeId == dwTargetNodeId && route.bConnected)
        {
            HRESULT hr = DisconnectRoute(route.dwRouteId);
            if (FAILED(hr))
                hrOverall = hr;
        }
    }
    return hrOverall;
}
