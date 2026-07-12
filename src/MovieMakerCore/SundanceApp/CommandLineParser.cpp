/*
 * CommandLineParser.cpp
 *
 * Implementation of the command-line argument parser for Sundance.
 * Processes switches and positional arguments from the launcher.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "CommandLineParser.h"

// ============================================================================
// Construction / destruction
// ============================================================================
CommandLineParser::CommandLineParser()
    : m_bHelp(false)
    , m_bSafeMode(false)
    , m_bCrashRecovery(false)
    , m_bPlay(false)
    , m_bPublish(false)
    , m_bImport(false)
    , m_bProjectOpen(false)
{
}

CommandLineParser::~CommandLineParser()
{
}

// ============================================================================
// Reset
// ============================================================================
void CommandLineParser::Reset()
{
    m_bHelp = false;
    m_bSafeMode = false;
    m_bCrashRecovery = false;
    m_bPlay = false;
    m_bPublish = false;
    m_bImport = false;
    m_bProjectOpen = false;

    m_strPlayFile.Empty();
    m_strPublishFile.Empty();
    m_strProjectFile.Empty();
    m_arrImportFiles.RemoveAll();
    m_strSqmSessionId.Empty();
    m_strDoneShowKey.Empty();
}

// ============================================================================
// IsSwitch / MatchSwitch
// ============================================================================
bool CommandLineParser::IsSwitch(LPCWSTR pszArg) const
{
    return pszArg && (pszArg[0] == L'/' || pszArg[0] == L'-');
}

bool CommandLineParser::MatchSwitch(LPCWSTR pszArg, LPCWSTR pszSwitch) const
{
    if (!pszArg || !pszSwitch)
        return false;

    // Skip leading '/' or '-'
    if (pszArg[0] == L'/' || pszArg[0] == L'-')
        ++pszArg;
    if (pszSwitch[0] == L'/' || pszSwitch[0] == L'-')
        ++pszSwitch;

    return _wcsicmp(pszArg, pszSwitch) == 0;
}

// ============================================================================
// Parse
// ============================================================================
void CommandLineParser::Parse(int argc, wchar_t** argv)
{
    Reset();

    if (argc <= 1 || !argv)
        return;

    // Skip argv[0] (the executable path)
    for (int i = 1; i < argc; ++i)
    {
        LPCWSTR pszArg = argv[i];
        if (!pszArg)
            continue;

        if (MatchSwitch(pszArg, L"help") || MatchSwitch(pszArg, L"?"))
        {
            m_bHelp = true;
            continue;
        }

        if (MatchSwitch(pszArg, L"safe"))
        {
            m_bSafeMode = true;
            continue;
        }

        if (MatchSwitch(pszArg, L"crashrecover"))
        {
            m_bCrashRecovery = true;
            continue;
        }

        if (MatchSwitch(pszArg, L"play"))
        {
            m_bPlay = true;
            if (i + 1 < argc && !IsSwitch(argv[i + 1]))
            {
                m_strPlayFile = argv[++i];
            }
            continue;
        }

        if (MatchSwitch(pszArg, L"publish"))
        {
            m_bPublish = true;
            if (i + 1 < argc && !IsSwitch(argv[i + 1]))
            {
                m_strPublishFile = argv[++i];
            }
            continue;
        }

        if (MatchSwitch(pszArg, L"import"))
        {
            m_bImport = true;
            // Collect all non-switch arguments until next switch or end
            while (i + 1 < argc && !IsSwitch(argv[i + 1]))
            {
                m_arrImportFiles.Add(ATL::CString(argv[++i]));
            }
            continue;
        }

        if (MatchSwitch(pszArg, L"project"))
        {
            m_bProjectOpen = true;
            if (i + 1 < argc && !IsSwitch(argv[i + 1]))
            {
                m_strProjectFile = argv[++i];
            }
            continue;
        }

        if (MatchSwitch(pszArg, L"sm"))
        {
            if (i + 1 < argc && !IsSwitch(argv[i + 1]))
            {
                m_strSqmSessionId = argv[++i];
            }
            continue;
        }

        if (MatchSwitch(pszArg, L"doneshow"))
        {
            if (i + 1 < argc && !IsSwitch(argv[i + 1]))
            {
                m_strDoneShowKey = argv[++i];
            }
            continue;
        }

        // Unknown switch or positional argument - treat as potential file
        // for import if no other mode was set
        if (!m_bPlay && !m_bPublish && !m_bProjectOpen)
        {
            if (!IsSwitch(pszArg))
            {
                m_arrImportFiles.Add(ATL::CString(pszArg));
                m_bImport = true;
            }
        }
    }
}

// ============================================================================
// Query methods
// ============================================================================
bool CommandLineParser::IsHelpRequested() const throw()
{
    return m_bHelp;
}

bool CommandLineParser::IsSafeMode() const throw()
{
    return m_bSafeMode;
}

bool CommandLineParser::IsCrashRecovery() const throw()
{
    return m_bCrashRecovery;
}

bool CommandLineParser::IsPlayRequested() const throw()
{
    return m_bPlay;
}

bool CommandLineParser::IsPublishRequested() const throw()
{
    return m_bPublish;
}

bool CommandLineParser::IsImportRequested() const throw()
{
    return m_bImport;
}

bool CommandLineParser::IsProjectOpenRequested() const throw()
{
    return m_bProjectOpen;
}

// ============================================================================
// Value accessors
// ============================================================================
LPCWSTR CommandLineParser::GetPlayFile() const throw()
{
    return m_strPlayFile;
}

LPCWSTR CommandLineParser::GetPublishFile() const throw()
{
    return m_strPublishFile;
}

LPCWSTR CommandLineParser::GetProjectFile() const throw()
{
    return m_strProjectFile;
}

const ATL::CStringArray& CommandLineParser::GetImportFiles() const throw()
{
    return m_arrImportFiles;
}

int CommandLineParser::GetImportFileCount() const throw()
{
    return static_cast<int>(m_arrImportFiles.GetCount());
}

LPCWSTR CommandLineParser::GetSqmSessionId() const throw()
{
    return m_strSqmSessionId;
}

LPCWSTR CommandLineParser::GetDoneShowKey() const throw()
{
    return m_strDoneShowKey;
}
