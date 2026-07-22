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
{
}

CommandBin::~CommandBin()
{
    Clear();
}

void CommandBin::AddCommand(const CommandEntry& entry)
{
    if (m_bCoalesceEnabled)
    {
        for (auto& existing : m_commands)
        {
            if (ShouldCoalesce(existing, entry))
            {
                existing = entry;
                m_bDirty = true;
                return;
            }
        }
    }
    m_commands.push_back(entry);
    m_bDirty = true;
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
    entry.strArgument = pszArgument ? pszArgument : L"";
    AddCommand(entry);
}

HRESULT CommandBin::Flush()
{
    if (!m_bDirty)
        return S_FALSE;

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

    m_commands.clear();
    m_bDirty = false;
    return hrOverall;
}

void CommandBin::Clear()
{
    m_commands.clear();
    m_bDirty = false;
}

size_t CommandBin::GetCommandCount() const throw()
{
    return m_commands.size();
}

bool CommandBin::IsDirty() const throw()
{
    return m_bDirty;
}

void CommandBin::EnableCoalescing(bool bEnable)
{
    m_bCoalesceEnabled = bEnable;
}

bool CommandBin::IsCoalescingEnabled() const throw()
{
    return m_bCoalesceEnabled;
}

void CommandBin::SetPriority(DWORD dwPriority)
{
    m_dwPriority = dwPriority;
}

DWORD CommandBin::GetPriority() const throw()
{
    return m_dwPriority;
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
    m_dwRenderTargetId = dwTargetId;
    AddCommand(CommandTypeUnknown, dwTargetId, static_cast<DWORD>(0));
}

void RenderCommandBin::SetViewport(const RECT& rcViewport)
{
    m_rcViewport = rcViewport;
}

void RenderCommandBin::SetShaderParameter(DWORD dwParamId, const float* pValues, DWORD dwCount)
{
    if (!pValues || dwCount == 0)
        return;

    ShaderParamEntry entry;
    entry.dwParamId = dwParamId;
    entry.values.assign(pValues, pValues + dwCount);
    m_shaderParams.push_back(std::move(entry));
    m_bDirty = true;
}

void RenderCommandBin::BindTexture(DWORD dwTextureId, DWORD dwSlot)
{
    TextureBinding binding;
    binding.dwTextureId = dwTextureId;
    binding.dwSlot = dwSlot;
    m_textureBindings.push_back(binding);
    m_bDirty = true;
}

void RenderCommandBin::SetClearColor(float flR, float flG, float flB, float flA)
{
    m_flClearColorR = flR;
    m_flClearColorG = flG;
    m_flClearColorB = flB;
    m_flClearColorA = flA;
    m_bDirty = true;
}

HRESULT RenderCommandBin::Flush()
{
    HRESULT hr = CommandBin::Flush();
    m_hrLastRenderResult = hr;
    return hr;
}

HRESULT RenderCommandBin::GetLastRenderResult() const throw()
{
    return m_hrLastRenderResult;
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
    entry.dwRouteId = m_dwNextRouteId++;
    entry.dwSourceNodeId = dwSourceNodeId;
    entry.strSourceField = pszSourceField;
    entry.dwTargetNodeId = dwTargetNodeId;
    entry.strTargetField = pszTargetField;
    entry.bConnected = false;

    m_routes.push_back(entry);
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
