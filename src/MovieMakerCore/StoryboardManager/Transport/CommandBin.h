/*
 * CommandBin.h
 *
 * Command buffer classes for batched scene graph operations.
 *
 * CommandBin: Command buffer for batched scene graph operations.
 * RenderCommandBin: Render-specific command buffer.
 * DynamicRouteManager: X3D route management (connect/disconnect routes).
 *
 * RTTI: ?AVCommandBin@@, ?AVRenderCommandBin@@, ?AVDynamicRouteManager@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef COMMANDBIN_H
#define COMMANDBIN_H

#include "../../pch.h"

// ============================================================================
// CommandType enum
// ============================================================================
enum CommandType
{
    CommandTypeUnknown           = 0,
    CommandTypeAddNode           = 1,
    CommandTypeRemoveNode        = 2,
    CommandTypeSetProperty       = 3,
    CommandTypeConnectRoute      = 4,
    CommandTypeDisconnectRoute   = 5,
    CommandTypeSetTransform      = 6,
    CommandTypeSetVisibility     = 7,
    CommandTypeSetOpacity        = 8,
    CommandTypePlayAnimation     = 9,
    CommandTypeStopAnimation     = 10,
    CommandTypeSetTimelineClock  = 11
};

// ============================================================================
// CommandEntry
// ============================================================================
// A single command entry in the CommandBin buffer.
//
struct CommandEntry
{
    CommandType    cmdType;
    DWORD          dwTargetId;
    DWORD          dwFlags;
    LONGLONG       llArgument;
    float          flArgument;
    ATL::CString   strArgument;

    CommandEntry()
        : cmdType(CommandTypeUnknown)
        , dwTargetId(0)
        , dwFlags(0)
        , llArgument(0)
        , flArgument(0.0f)
    {
    }
};

// ============================================================================
// CommandBin
// ============================================================================
// Buffer for batched scene graph operations. Commands are accumulated into
// the buffer and then flushed in a batch to minimize X3D scene graph updates.
// Supports priority ordering, coalescing of duplicate operations, and
// transactional commit/rollback.
//
class CommandBin
{
public:
    CommandBin();
    ~CommandBin();

    // -- Command queuing --
    void AddCommand(const CommandEntry& entry);
    void AddCommand(CommandType cmdType, DWORD dwTargetId, DWORD dwFlags = 0);
    void AddCommand(CommandType cmdType, DWORD dwTargetId, LONGLONG llArgument);
    void AddCommand(CommandType cmdType, DWORD dwTargetId, float flArgument);
    void AddCommand(CommandType cmdType, DWORD dwTargetId, LPCWSTR pszArgument);

    // -- Batch operations --
    HRESULT Flush();
    void    Clear();
    size_t  GetCommandCount() const throw();
    bool    IsDirty() const throw();

    // -- Coalescing --
    void EnableCoalescing(bool bEnable);
    bool IsCoalescingEnabled() const throw();

    // -- Priority --
    void SetPriority(DWORD dwPriority);
    DWORD GetPriority() const throw();

protected:
    bool ShouldCoalesce(const CommandEntry& existing, const CommandEntry& incoming);

    std::vector<CommandEntry> m_commands;
    bool                      m_bCoalesceEnabled;
    bool                      m_bDirty;
    DWORD                     m_dwPriority;
};

// ============================================================================
// RenderCommandBin
// ============================================================================
// Render-specific command buffer. Extends CommandBin with render-specific
// commands such as texture binding, shader parameter updates, and render
// target changes. Used during the encode/export pipeline.
//
class RenderCommandBin : public CommandBin
{
public:
    RenderCommandBin();
    ~RenderCommandBin();

    // -- Render-specific commands --
    void SetRenderTarget(DWORD dwTargetId);
    void SetViewport(const RECT& rcViewport);
    void SetShaderParameter(DWORD dwParamId, const float* pValues, DWORD dwCount);
    void BindTexture(DWORD dwTextureId, DWORD dwSlot);
    void SetClearColor(float flR, float flG, float flB, float flA);

    // -- Batch execution --
    HRESULT Flush() override;

    // -- Result --
    HRESULT GetLastRenderResult() const throw();

private:
    RECT    m_rcViewport;
    DWORD   m_dwRenderTargetId;
    HRESULT m_hrLastRenderResult;
};

// ============================================================================
// DynamicRouteManager
// ============================================================================
// Manages X3D scene graph route connections. Routes define the flow of
// data between scene nodes (e.g., time sensors driving interpolators which
// drive transform nodes). Supports dynamic connect/disconnect of routes
// at runtime to update the scene graph without full topology rebuild.
//
class DynamicRouteManager
{
public:
    DynamicRouteManager();
    ~DynamicRouteManager();

    // -- Route definition --
    struct RouteEntry
    {
        DWORD       dwRouteId;
        DWORD       dwSourceNodeId;
        ATL::CString strSourceField;
        DWORD       dwTargetNodeId;
        ATL::CString strTargetField;
        bool        bConnected;

        RouteEntry()
            : dwRouteId(0)
            , dwSourceNodeId(0)
            , dwTargetNodeId(0)
            , bConnected(false)
        {
        }
    };

    // -- Route management --
    HRESULT AddRoute(DWORD dwSourceNodeId, LPCWSTR pszSourceField,
                     DWORD dwTargetNodeId, LPCWSTR pszTargetField);
    HRESULT RemoveRoute(DWORD dwRouteId);
    HRESULT ConnectRoute(DWORD dwRouteId);
    HRESULT DisconnectRoute(DWORD dwRouteId);
    HRESULT DisconnectAllRoutes();
    HRESULT ConnectAllRoutes();

    // -- Query --
    size_t GetRouteCount() const throw();
    const RouteEntry* GetRoute(DWORD dwRouteId) const;
    const RouteEntry* GetRouteAt(size_t nIndex) const;

    // -- Bulk operations --
    HRESULT ConnectRoutesBySource(DWORD dwSourceNodeId);
    HRESULT ConnectRoutesByTarget(DWORD dwTargetNodeId);
    HRESULT DisconnectRoutesBySource(DWORD dwSourceNodeId);
    HRESULT DisconnectRoutesByTarget(DWORD dwTargetNodeId);

private:
    DWORD                    m_dwNextRouteId;
    std::vector<RouteEntry>  m_routes;
};

#endif // COMMANDBIN_H
