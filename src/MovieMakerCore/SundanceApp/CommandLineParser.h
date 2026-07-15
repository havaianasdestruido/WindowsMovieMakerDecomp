/*
 * CommandLineParser.h
 *
 * Command-line argument parser for the Sundance application framework.
 * Processes command-line switches and arguments passed from the launcher
 * (MovieMaker.exe) through to MovieMakerMain.
 *
 * RTTI: ?AVCommandLineParser@@
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_COMMAND_LINE_PARSER_H
#define SUNDANCE_COMMAND_LINE_PARSER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

// ============================================================================
// CommandLineParser
// ============================================================================
// Parses the argc/argv passed to MovieMakerMain and extracts recognized
// switches and positional arguments. The parsed results are queried by
// SundanceAppMain::ProcessCommandLine to drive application behavior.
//
// Recognized switches (from disassembly / string analysis):
//   /play <file>         - Open and play a media file
//   /publish <file>      - Open and publish a project
//   /import <file>       - Import media files
//   /project <file>      - Open a project file
//   /safe                - Start in safe mode (no DirectUI)
//   /crashrecover        - Attempt crash recovery
//   /sm [guid]           - SQM telemetry session
//   /doneshow <key>      - Mark a DontShow prompt as dismissed
//   /?, /help            - Show usage
//
class CommandLineParser
{
public:
    CommandLineParser();
    ~CommandLineParser();

    // -- Parse entry point --
    void Parse(int argc, wchar_t** argv);

    // -- Query parsed results --
    bool IsHelpRequested() const throw();
    bool IsSafeMode() const throw();
    bool IsCrashRecovery() const throw();
    bool IsPlayRequested() const throw();
    bool IsPublishRequested() const throw();
    bool IsImportRequested() const throw();
    bool IsProjectOpenRequested() const throw();

    // -- Retrieve parsed values --
    LPCWSTR GetPlayFile() const throw();
    LPCWSTR GetPublishFile() const throw();
    LPCWSTR GetProjectFile() const throw();
    const std::vector<ATL::CString>& GetImportFiles() const throw();
    LPCWSTR GetSqmSessionId() const throw();
    LPCWSTR GetDoneShowKey() const throw();

    // -- File count for import --
    int GetImportFileCount() const throw();

private:
    // Flags
    bool    m_bHelp;
    bool    m_bSafeMode;
    bool    m_bCrashRecovery;
    bool    m_bPlay;
    bool    m_bPublish;
    bool    m_bImport;
    bool    m_bProjectOpen;

    // File paths
    ATL::CString   m_strPlayFile;
    ATL::CString   m_strPublishFile;
    ATL::CString   m_strProjectFile;
    std::vector<ATL::CString> m_arrImportFiles;

    // Telemetry / prompt keys
    ATL::CString   m_strSqmSessionId;
    ATL::CString   m_strDoneShowKey;

    // Internal helpers
    void Reset();
    bool IsSwitch(LPCWSTR pszArg) const;
    bool MatchSwitch(LPCWSTR pszArg, LPCWSTR pszSwitch) const;
};

#endif // SUNDANCE_COMMAND_LINE_PARSER_H
