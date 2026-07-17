#include "pch.h"
/*
 * Conductor.cpp
 *
 * Implementation of Conductor, ThemeOperationLogger, MonolithicThemeOperation,
 * and MTOListContainer.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "Conductor.h"
#include "AutofitProcess.h"

// ============================================================================
// MonolithicThemeOperation implementation
// ============================================================================

MonolithicThemeOperation::MonolithicThemeOperation()
    : m_dwOperationId(0)
    , m_bInProgress(false)
    , m_bCompleted(false)
    , m_bFailed(false)
{
    ZeroMemory(&m_ftStartTime, sizeof(FILETIME));
    ZeroMemory(&m_ftEndTime, sizeof(FILETIME));
}

MonolithicThemeOperation::~MonolithicThemeOperation()
{
}

void MonolithicThemeOperation::SetOperationId(DWORD dwId)
{
    m_dwOperationId = dwId;
}

DWORD MonolithicThemeOperation::GetOperationId() const throw()
{
    return m_dwOperationId;
}

void MonolithicThemeOperation::SetOperationName(LPCWSTR pszName)
{
    m_strOperationName = pszName ? pszName : L"";
}

ATL::CString MonolithicThemeOperation::GetOperationName() const
{
    return m_strOperationName;
}

void MonolithicThemeOperation::AddStep(LPCWSTR pszStepDescription)
{
    if (pszStepDescription)
        m_vSteps.push_back(pszStepDescription);
}

size_t MonolithicThemeOperation::GetStepCount() const throw()
{
    return m_vSteps.size();
}

ATL::CString MonolithicThemeOperation::GetStepAt(size_t nIndex) const
{
    if (nIndex < m_vSteps.size())
        return m_vSteps[nIndex];
    return L"";
}

void MonolithicThemeOperation::SetInProgress(bool bInProgress)
{
    m_bInProgress = bInProgress;
}

bool MonolithicThemeOperation::IsInProgress() const throw()
{
    return m_bInProgress;
}

void MonolithicThemeOperation::SetCompleted(bool bCompleted)
{
    m_bCompleted = bCompleted;
}

bool MonolithicThemeOperation::IsCompleted() const throw()
{
    return m_bCompleted;
}

void MonolithicThemeOperation::SetFailed(bool bFailed)
{
    m_bFailed = bFailed;
}

bool MonolithicThemeOperation::IsFailed() const throw()
{
    return m_bFailed;
}

void MonolithicThemeOperation::SetErrorDescription(LPCWSTR pszError)
{
    m_strErrorDescription = pszError ? pszError : L"";
}

ATL::CString MonolithicThemeOperation::GetErrorDescription() const
{
    return m_strErrorDescription;
}

FILETIME MonolithicThemeOperation::GetStartTime() const throw()
{
    return m_ftStartTime;
}

void MonolithicThemeOperation::SetStartTime(const FILETIME& ft)
{
    m_ftStartTime = ft;
}

FILETIME MonolithicThemeOperation::GetEndTime() const throw()
{
    return m_ftEndTime;
}

void MonolithicThemeOperation::SetEndTime(const FILETIME& ft)
{
    m_ftEndTime = ft;
}

// ============================================================================
// MTOListContainer implementation
// ============================================================================

MTOListContainer::MTOListContainer()
{
}

MTOListContainer::~MTOListContainer()
{
    RemoveAllOperations();
}

void MTOListContainer::AddOperation(MonolithicThemeOperation* pOperation)
{
    if (pOperation)
        m_vOperations.push_back(pOperation);
}

void MTOListContainer::RemoveOperation(DWORD dwOperationId)
{
    for (auto it = m_vOperations.begin(); it != m_vOperations.end(); ++it)
    {
        if ((*it)->GetOperationId() == dwOperationId)
        {
            delete *it;
            m_vOperations.erase(it);
            return;
        }
    }
}

void MTOListContainer::RemoveAllOperations()
{
    for (auto pOp : m_vOperations)
        delete pOp;
    m_vOperations.clear();
}

size_t MTOListContainer::GetOperationCount() const throw()
{
    return m_vOperations.size();
}

MonolithicThemeOperation* MTOListContainer::GetOperation(DWORD dwOperationId) const
{
    for (auto pOp : m_vOperations)
    {
        if (pOp->GetOperationId() == dwOperationId)
            return pOp;
    }
    return nullptr;
}

MonolithicThemeOperation* MTOListContainer::GetOperationAt(size_t nIndex) const
{
    if (nIndex < m_vOperations.size())
        return m_vOperations[nIndex];
    return nullptr;
}

bool MTOListContainer::HasPendingOperations() const throw()
{
    for (auto pOp : m_vOperations)
    {
        if (!pOp->IsCompleted() && !pOp->IsFailed())
            return true;
    }
    return false;
}

bool MTOListContainer::HasFailedOperations() const throw()
{
    for (auto pOp : m_vOperations)
    {
        if (pOp->IsFailed())
            return true;
    }
    return false;
}

// ============================================================================
// ThemeOperationLogger implementation
// ============================================================================

ThemeOperationLogger::ThemeOperationLogger()
{
    InitializeCriticalSection(&m_csLog);
}

ThemeOperationLogger::~ThemeOperationLogger()
{
    ClearLog();
    DeleteCriticalSection(&m_csLog);
}

void ThemeOperationLogger::LogOperation(MonolithicThemeOperation* pOperation)
{
    if (!pOperation)
        return;

    EnterCriticalSection(&m_csLog);
    m_vLogEntries.push_back(pOperation);
    LeaveCriticalSection(&m_csLog);
}

void ThemeOperationLogger::ClearLog()
{
    EnterCriticalSection(&m_csLog);
    for (auto pEntry : m_vLogEntries)
        delete pEntry;
    m_vLogEntries.clear();
    LeaveCriticalSection(&m_csLog);
}

size_t ThemeOperationLogger::GetLogEntryCount() const throw()
{
    EnterCriticalSection(&m_csLog);
    size_t count = m_vLogEntries.size();
    LeaveCriticalSection(&m_csLog);
    return count;
}

MonolithicThemeOperation* ThemeOperationLogger::FindOperation(DWORD dwOperationId) const
{
    EnterCriticalSection(&m_csLog);
    for (auto pEntry : m_vLogEntries)
    {
        if (pEntry->GetOperationId() == dwOperationId)
        {
            LeaveCriticalSection(&m_csLog);
            return pEntry;
        }
    }
    LeaveCriticalSection(&m_csLog);
    return nullptr;
}

MonolithicThemeOperation* ThemeOperationLogger::FindOperationByName(LPCWSTR pszName) const
{
    if (!pszName)
        return nullptr;

    EnterCriticalSection(&m_csLog);
    for (auto pEntry : m_vLogEntries)
    {
        if (pEntry->GetOperationName().CompareNoCase(pszName) == 0)
        {
            LeaveCriticalSection(&m_csLog);
            return pEntry;
        }
    }
    LeaveCriticalSection(&m_csLog);
    return nullptr;
}

void ThemeOperationLogger::GetOperationsByStatus(
    bool bCompleted, std::vector<MonolithicThemeOperation*>& vResults) const
{
    vResults.clear();
    EnterCriticalSection(&m_csLog);
    for (auto pEntry : m_vLogEntries)
    {
        if (bCompleted ? pEntry->IsCompleted() : !pEntry->IsCompleted())
            vResults.push_back(pEntry);
    }
    LeaveCriticalSection(&m_csLog);
}

void ThemeOperationLogger::GetOperationsSince(
    const FILETIME& ftSince, std::vector<MonolithicThemeOperation*>& vResults) const
{
    vResults.clear();
    EnterCriticalSection(&m_csLog);
    for (auto pEntry : m_vLogEntries)
    {
        FILETIME ftStart = pEntry->GetStartTime();
        if (CompareFileTime(&ftStart, &ftSince) >= 0)
            vResults.push_back(pEntry);
    }
    LeaveCriticalSection(&m_csLog);
}

HRESULT ThemeOperationLogger::SaveToFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_POINTER;

    CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFile(pszFilePath, STGM_WRITE | STGM_CREATE, &spStream);
    if (FAILED(hr))
        return hr;

    CComPtr<IXmlWriter> spWriter;
    hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&spWriter), NULL);
    if (FAILED(hr))
        return hr;

    hr = spWriter->SetOutput(spStream);
    if (FAILED(hr))
        return hr;

    hr = spWriter->WriteStartDocument(XmlStandalone_Omit);
    if (FAILED(hr))
        return hr;

    EnterCriticalSection(&m_csLog);

    hr = spWriter->WriteStartElement(NULL, L"OperationLog", NULL);
    if (FAILED(hr))
    {
        LeaveCriticalSection(&m_csLog);
        return hr;
    }

    WCHAR szCount[32];
    hr = StringCchPrintfW(szCount, _countof(szCount), L"%zu", m_vLogEntries.size());
    if (SUCCEEDED(hr))
    {
        spWriter->WriteAttributeString(NULL, L"count", NULL, szCount);
    }

    for (auto pEntry : m_vLogEntries)
    {
        hr = spWriter->WriteStartElement(NULL, L"Operation", NULL);
        if (FAILED(hr))
            break;

        WCHAR szId[32];
        StringCchPrintfW(szId, _countof(szId), L"%u", pEntry->GetOperationId());
        spWriter->WriteAttributeString(NULL, L"id", NULL, szId);

        spWriter->WriteAttributeString(NULL, L"name", NULL, pEntry->GetOperationName());

        if (pEntry->IsCompleted())
            spWriter->WriteAttributeString(NULL, L"status", NULL, L"Completed");
        else if (pEntry->IsFailed())
            spWriter->WriteAttributeString(NULL, L"status", NULL, L"Failed");
        else if (pEntry->IsInProgress())
            spWriter->WriteAttributeString(NULL, L"status", NULL, L"InProgress");
        else
            spWriter->WriteAttributeString(NULL, L"status", NULL, L"Pending");

        FILETIME ftStart = pEntry->GetStartTime();
        FILETIME ftEnd = pEntry->GetEndTime();
        ULARGE_INTEGER uliStart, uliEnd;
        uliStart.LowPart = ftStart.dwLowDateTime;
        uliStart.HighPart = ftStart.dwHighDateTime;
        uliEnd.LowPart = ftEnd.dwLowDateTime;
        uliEnd.HighPart = ftEnd.dwHighDateTime;

        WCHAR szStart[32], szEnd[32];
        StringCchPrintfW(szStart, _countof(szStart), L"%llu", uliStart.QuadPart);
        StringCchPrintfW(szEnd, _countof(szEnd), L"%llu", uliEnd.QuadPart);
        spWriter->WriteAttributeString(NULL, L"startTime", NULL, szStart);
        spWriter->WriteAttributeString(NULL, L"endTime", NULL, szEnd);

        if (pEntry->IsFailed())
        {
            spWriter->WriteAttributeString(NULL, L"error", NULL, pEntry->GetErrorDescription());
        }

        size_t stepCount = pEntry->GetStepCount();
        if (stepCount > 0)
        {
            spWriter->WriteStartElement(NULL, L"Steps", NULL);
            for (size_t s = 0; s < stepCount; s++)
            {
                spWriter->WriteStartElement(NULL, L"Step", NULL);
                spWriter->WriteString(pEntry->GetStepAt(s));
                spWriter->WriteEndElement();
            }
            spWriter->WriteEndElement();
        }

        spWriter->WriteEndElement();
    }

    LeaveCriticalSection(&m_csLog);

    if (FAILED(hr))
        return hr;

    hr = spWriter->WriteEndElement();
    if (FAILED(hr))
        return hr;

    hr = spWriter->WriteEndDocument();
    if (FAILED(hr))
        return hr;

    hr = spWriter->Flush();
    return hr;
}

HRESULT ThemeOperationLogger::LoadFromFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_POINTER;

    CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFile(pszFilePath, STGM_READ, &spStream);
    if (FAILED(hr))
        return hr;

    CComPtr<IXmlReader> spReader;
    hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&spReader), NULL);
    if (FAILED(hr))
        return hr;

    hr = spReader->SetInput(spStream);
    if (FAILED(hr))
        return hr;

    XmlNodeType nodeType;
    while (spReader->Read(&nodeType) == S_OK)
    {
        if (nodeType != XmlNodeType_Element)
            continue;

        const WCHAR* pwszLocalName = NULL;
        hr = spReader->GetLocalName(&pwszLocalName, NULL);
        if (FAILED(hr) || !pwszLocalName)
            continue;

        if (wcscmp(pwszLocalName, L"Operation") != 0)
            continue;

        MonolithicThemeOperation* pOp = new MonolithicThemeOperation();

        const WCHAR* pValue = NULL;

        if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"id", &pValue)) && pValue)
        {
            pOp->SetOperationId((DWORD)wcstoul(pValue, NULL, 10));
        }

        if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"name", &pValue)) && pValue)
        {
            pOp->SetOperationName(pValue);
        }

        if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"status", &pValue)) && pValue)
        {
            if (wcscmp(pValue, L"Completed") == 0)
            {
                pOp->SetCompleted(true);
            }
            else if (wcscmp(pValue, L"Failed") == 0)
            {
                pOp->SetFailed(true);
            }
            else if (wcscmp(pValue, L"InProgress") == 0)
            {
                pOp->SetInProgress(true);
            }
        }

        if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"startTime", &pValue)) && pValue)
        {
            FILETIME ft;
            ULARGE_INTEGER uli;
            uli.QuadPart = _wcstoui64(pValue, NULL, 10);
            ft.dwLowDateTime = uli.LowPart;
            ft.dwHighDateTime = uli.HighPart;
            pOp->SetStartTime(ft);
        }

        if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"endTime", &pValue)) && pValue)
        {
            FILETIME ft;
            ULARGE_INTEGER uli;
            uli.QuadPart = _wcstoui64(pValue, NULL, 10);
            ft.dwLowDateTime = uli.LowPart;
            ft.dwHighDateTime = uli.HighPart;
            pOp->SetEndTime(ft);
        }

        if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"error", &pValue)) && pValue)
        {
            pOp->SetErrorDescription(pValue);
        }

        EnterCriticalSection(&m_csLog);
        m_vLogEntries.push_back(pOp);
        LeaveCriticalSection(&m_csLog);
    }

    return S_OK;
}

// ============================================================================
// Conductor implementation
// ============================================================================

Conductor::Conductor()
    : m_pOrchestrator(nullptr)
    , m_pOperationLogger(nullptr)
    , m_pMTOContainer(nullptr)
    , m_bInitialized(false)
{
}

Conductor::~Conductor()
{
    Shutdown();
}

HRESULT Conductor::Initialize()
{
    if (m_bInitialized)
        return S_FALSE;

    m_pOrchestrator = new BackgroundOrchestrator();
    HRESULT hr = m_pOrchestrator->Initialize(4);
    if (FAILED(hr))
    {
        delete m_pOrchestrator;
        m_pOrchestrator = nullptr;
        return hr;
    }

    m_pOperationLogger = new ThemeOperationLogger();
    m_pMTOContainer = new MTOListContainer();

    m_bInitialized = true;
    return S_OK;
}

void Conductor::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_pOrchestrator)
    {
        m_pOrchestrator->Shutdown();
        delete m_pOrchestrator;
        m_pOrchestrator = nullptr;
    }

    delete m_pOperationLogger;
    m_pOperationLogger = nullptr;

    delete m_pMTOContainer;
    m_pMTOContainer = nullptr;

    m_bInitialized = false;
}

bool Conductor::IsInitialized() const throw()
{
    return m_bInitialized;
}

HRESULT Conductor::QueueTranscode(DWORD dwExtentId, DWORD dwMediaId)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    TranscodeBackgroundRequest* pRequest =
        new TranscodeBackgroundRequest(dwExtentId, dwMediaId);

    return m_pOrchestrator->QueueRequest(pRequest);
}

HRESULT Conductor::QueueMediaLoad(LPCWSTR pszFilePath)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    if (!pszFilePath)
        return E_POINTER;

    MediaLoadBackgroundRequest* pRequest =
        new MediaLoadBackgroundRequest(pszFilePath);

    return m_pOrchestrator->QueueRequest(pRequest);
}

HRESULT Conductor::QueueSceneMerge(SceneMergeContext* pContext)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    if (!pContext)
        return E_POINTER;

    SceneMergeBackgroundRequest* pRequest =
        new SceneMergeBackgroundRequest(pContext);

    return m_pOrchestrator->QueueRequest(pRequest);
}

HRESULT Conductor::BeginThemeOperation(LPCWSTR pszName, DWORD* pdwOperationId)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    if (!pszName || !pdwOperationId)
        return E_POINTER;

    MonolithicThemeOperation* pOp = new MonolithicThemeOperation();
    pOp->SetOperationName(pszName);

    static DWORD s_dwNextOpId = 1;
    pOp->SetOperationId(s_dwNextOpId++);

    FILETIME ftNow;
    GetSystemTimeAsFileTime(&ftNow);
    pOp->SetStartTime(ftNow);
    pOp->SetInProgress(true);

    m_pMTOContainer->AddOperation(pOp);
    m_pOperationLogger->LogOperation(pOp);

    *pdwOperationId = pOp->GetOperationId();
    return S_OK;
}

HRESULT Conductor::CompleteThemeOperation(DWORD dwOperationId)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    MonolithicThemeOperation* pOp = m_pMTOContainer->GetOperation(dwOperationId);
    if (!pOp)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    FILETIME ftNow;
    GetSystemTimeAsFileTime(&ftNow);
    pOp->SetEndTime(ftNow);
    pOp->SetInProgress(false);
    pOp->SetCompleted(true);

    return S_OK;
}

HRESULT Conductor::FailThemeOperation(DWORD dwOperationId, LPCWSTR pszError)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    MonolithicThemeOperation* pOp = m_pMTOContainer->GetOperation(dwOperationId);
    if (!pOp)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    FILETIME ftNow;
    GetSystemTimeAsFileTime(&ftNow);
    pOp->SetEndTime(ftNow);
    pOp->SetInProgress(false);
    pOp->SetFailed(true);
    pOp->SetErrorDescription(pszError);

    return S_OK;
}

HRESULT Conductor::RunAutofit(DWORD dwPhotoCount, LONGLONG llTotalDurationHns)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    AutofitProcess autofit;
    autofit.SetPhotoCount(dwPhotoCount);
    autofit.SetTotalDurationHns(llTotalDurationHns);

    return autofit.Analyze();
}

BackgroundOrchestrator* Conductor::GetOrchestrator() const throw()
{
    return m_pOrchestrator;
}

ThemeOperationLogger* Conductor::GetOperationLogger() const throw()
{
    return m_pOperationLogger;
}
