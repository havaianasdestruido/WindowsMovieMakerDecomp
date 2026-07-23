#include "pch.h"
/*
 * SerializationWriter.cpp
 *
 * Implementation of the .wlmp project file writer using XmlLite.
 * Produces well-formed XML output with project structure, media items,
 * timelines, transitions, effects, titles, and settings.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "SerializationWriter.h"
#include "../MovieProject.h"
#include <strsafe.h>

namespace StoryboardManager
{

// ============================================================================
// Construction / destruction
// ============================================================================

SerializationWriter::SerializationWriter()
    : m_pWriter(nullptr)
    , m_pStream(nullptr)
    , m_nIndentLevel(0)
    , m_dwElementDepth(0)
    , m_hrLast(S_OK)
{
    m_ctx = SerializationContext::CreateForWrite();
}

SerializationWriter::~SerializationWriter()
{
    if (m_pWriter)
    {
        m_pWriter->Release();
        m_pWriter = nullptr;
    }
}

// ============================================================================
// WriteToFile
// ============================================================================

HRESULT SerializationWriter::WriteToFile(LPCWSTR pszFilePath, MovieProject* pProject)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    if (!pProject)
        return E_POINTER;

    IStream* pStream = nullptr;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_WRITE | STGM_CREATE, &pStream);
    if (FAILED(hr))
        return hr;

    hr = WriteToStream(pStream, pProject);
    pStream->Release();

    return hr;
}

// ============================================================================
// WriteToStream
// ============================================================================

HRESULT SerializationWriter::WriteToStream(IStream* pStream, MovieProject* pProject)
{
    if (!pStream)
        return E_POINTER;

    if (!pProject)
        return E_POINTER;

    HRESULT hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&m_pWriter), nullptr);
    if (FAILED(hr))
        return hr;

    m_pStream = pStream;
    m_ctx.Reset();

    hr = m_pWriter->SetOutput(pStream);
    if (SUCCEEDED(hr))
        hr = BeginDocument();

    if (SUCCEEDED(hr))
        hr = WriteProjectElement(pProject);

    if (SUCCEEDED(hr))
        hr = EndDocument();

    if (SUCCEEDED(hr))
        hr = m_pWriter->Flush();

    m_hrLast = hr;
    return hr;
}

// ============================================================================
// Element nesting
// ============================================================================

HRESULT SerializationWriter::BeginElement(LPCWSTR pszName)
{
    if (!m_pWriter || !pszName)
        return E_UNEXPECTED;

    HRESULT hr = m_pWriter->WriteStartElement(nullptr, pszName, nullptr);
    if (SUCCEEDED(hr))
    {
        ++m_dwElementDepth;
        ++m_nIndentLevel;
    }

    m_hrLast = hr;
    return hr;
}

HRESULT SerializationWriter::EndElement()
{
    if (!m_pWriter)
        return E_UNEXPECTED;

    HRESULT hr = m_pWriter->WriteEndElement();
    if (SUCCEEDED(hr))
    {
        if (m_dwElementDepth > 0)
            --m_dwElementDepth;
        if (m_nIndentLevel > 0)
            --m_nIndentLevel;
    }

    m_hrLast = hr;
    return hr;
}

HRESULT SerializationWriter::BeginElementWithId(LPCWSTR pszName, DWORD dwId)
{
    HRESULT hr = BeginElement(pszName);
    if (SUCCEEDED(hr))
    {
        WCHAR szBuf[64];
        hr = WriteAttribute(L"id", _itow_s(dwId, szBuf, 10));
    }
    return hr;
}

// ============================================================================
// Attribute writing (typed)
// ============================================================================

HRESULT SerializationWriter::WriteAttribute(LPCWSTR pszName, LPCWSTR pszValue)
{
    if (!m_pWriter || !pszName)
        return E_UNEXPECTED;

    HRESULT hr = m_pWriter->WriteAttributeString(nullptr, pszName, nullptr,
        pszValue ? pszValue : L"");
    m_hrLast = hr;
    return hr;
}

HRESULT SerializationWriter::WriteAttribute(LPCWSTR pszName, int nValue)
{
    WCHAR szBuf[64];
    _itow_s(nValue, szBuf, 10);
    return WriteAttribute(pszName, szBuf);
}

HRESULT SerializationWriter::WriteAttribute(LPCWSTR pszName, DWORD dwValue)
{
    WCHAR szBuf[64];
    _itow_s(dwValue, szBuf, 10);
    return WriteAttribute(pszName, szBuf);
}

HRESULT SerializationWriter::WriteAttribute(LPCWSTR pszName, LONGLONG llValue)
{
    WCHAR szBuf[64];
    _i64tow_s(llValue, szBuf, _countof(szBuf), 10);
    return WriteAttribute(pszName, szBuf);
}

HRESULT SerializationWriter::WriteAttribute(LPCWSTR pszName, double dblValue)
{
    WCHAR szBuf[64];
    _snwprintf_s(szBuf, _countof(szBuf), _TRUNCATE, L"%g", dblValue);
    return WriteAttribute(pszName, szBuf);
}

HRESULT SerializationWriter::WriteAttribute(LPCWSTR pszName, bool fValue)
{
    return WriteAttribute(pszName, fValue ? L"true" : L"false");
}

// ============================================================================
// Text content
// ============================================================================

HRESULT SerializationWriter::WriteText(LPCWSTR pszText)
{
    if (!m_pWriter)
        return E_UNEXPECTED;

    HRESULT hr = m_pWriter->WriteString(pszText ? pszText : L"");
    m_hrLast = hr;
    return hr;
}

// ============================================================================
// Document lifecycle
// ============================================================================

HRESULT SerializationWriter::BeginDocument()
{
    if (!m_pWriter)
        return E_UNEXPECTED;

    HRESULT hr = m_pWriter->WriteStartDocument(XmlStandalone_Omit);
    m_hrLast = hr;
    return hr;
}

HRESULT SerializationWriter::EndDocument()
{
    if (!m_pWriter)
        return E_UNEXPECTED;

    HRESULT hr = m_pWriter->WriteEndDocument();
    m_hrLast = hr;
    return hr;
}

HRESULT SerializationWriter::Flush()
{
    if (!m_pWriter)
        return E_UNEXPECTED;

    HRESULT hr = m_pWriter->Flush();
    m_hrLast = hr;
    return hr;
}

// ============================================================================
// Indentation
// ============================================================================

void SerializationWriter::SetIndentLevel(int nLevel)
{
    m_nIndentLevel = nLevel;
}

int SerializationWriter::GetIndentLevel() const throw()
{
    return m_nIndentLevel;
}

// ============================================================================
// Error state / context access
// ============================================================================

HRESULT SerializationWriter::GetLastHResult() const throw()
{
    return m_hrLast;
}

SerializationContext& SerializationWriter::GetContext()
{
    return m_ctx;
}

const SerializationContext& SerializationWriter::GetContext() const
{
    return m_ctx;
}

// ============================================================================
// Project serialization
// ============================================================================

HRESULT SerializationWriter::WriteProjectElement(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    HRESULT hr;

    hr = BeginElement(L"project");
    if (FAILED(hr)) return hr;

    hr = WriteAttribute(L"versionMajor", static_cast<int>(pProject->GetVersionMajor()));
    if (FAILED(hr)) return hr;

    hr = WriteAttribute(L"versionMinor", static_cast<int>(pProject->GetVersionMinor()));
    if (FAILED(hr)) return hr;

    if (!pProject->GetProjectName().IsEmpty())
    {
        hr = WriteAttribute(L"name", pProject->GetProjectName());
        if (FAILED(hr)) return hr;
    }

    hr = WriteMediaElement(pProject);
    if (SUCCEEDED(hr))
        hr = WriteTimelineElement(pProject);
    if (SUCCEEDED(hr))
        hr = WriteTransitionsElement(pProject);
    if (SUCCEEDED(hr))
        hr = WriteEffectsElement(pProject);
    if (SUCCEEDED(hr))
        hr = WriteTitlesElement(pProject);
    if (SUCCEEDED(hr))
        hr = WriteSettingsElement(pProject);

    if (SUCCEEDED(hr))
        hr = EndElement();

    return hr;
}

HRESULT SerializationWriter::WriteMediaElement(MovieProject* pProject)
{
    HRESULT hr = BeginElement(L"media");
    if (FAILED(hr)) return hr;

    for (size_t i = 0; i < pProject->GetMediaItemCount(); ++i)
    {
        const ProjectMediaItem* pItem = pProject->GetMediaItem(i);
        if (!pItem)
            continue;

        hr = BeginElement(L"mediaItem");
        if (FAILED(hr)) break;

        hr = WriteAttribute(L"id", pItem->GetMediaId());
        if (FAILED(hr)) break;

        hr = WriteAttribute(L"path", pItem->GetSourcePath());
        if (FAILED(hr)) break;

        hr = WriteAttribute(L"type", pItem->GetMediaType());
        if (FAILED(hr)) break;

        hr = WriteAttribute(L"duration", pItem->GetDurationHns());
        if (FAILED(hr)) break;

        if (pItem->GetWidth() > 0 && pItem->GetHeight() > 0)
        {
            hr = WriteAttribute(L"width", static_cast<int>(pItem->GetWidth()));
            if (FAILED(hr)) break;

            hr = WriteAttribute(L"height", static_cast<int>(pItem->GetHeight()));
            if (FAILED(hr)) break;
        }

        if (pItem->GetFrameRate() > 0)
        {
            hr = WriteAttribute(L"frameRate", pItem->GetFrameRate());
            if (FAILED(hr)) break;
        }

        if (SUCCEEDED(hr))
            hr = EndElement();
        if (FAILED(hr)) break;
    }

    if (SUCCEEDED(hr))
        hr = EndElement();

    return hr;
}

HRESULT SerializationWriter::WriteTimelineElement(MovieProject* pProject)
{
    static const LPCWSTR trackNames[] = {
        L"video", L"audio", L"music", L"title", L"credits", L"transition"
    };

    HRESULT hr = S_OK;

    for (int t = 0; t < 6; ++t)
    {
        const ProjectTimeline* pTimeline = pProject->GetTimeline(
            static_cast<TimelineTrackType>(t));
        if (!pTimeline || pTimeline->GetExtentCount() == 0)
            continue;

        hr = BeginElement(L"timeline");
        if (FAILED(hr)) break;

        hr = WriteAttribute(L"track", trackNames[t]);
        if (FAILED(hr)) break;

        for (size_t e = 0; e < pTimeline->GetExtentCount(); ++e)
        {
            hr = BeginElement(L"extent");
            if (FAILED(hr)) break;

            hr = WriteAttribute(L"id", static_cast<DWORD>(pTimeline->GetExtentIdAt(e)));
            if (FAILED(hr)) break;

            hr = EndElement();
            if (FAILED(hr)) break;
        }

        if (SUCCEEDED(hr))
            hr = EndElement();
        if (FAILED(hr)) break;
    }

    return hr;
}

HRESULT SerializationWriter::WriteTransitionsElement(MovieProject* pProject)
{
    HRESULT hr = BeginElement(L"transitions");
    if (FAILED(hr)) return hr;

    const ProjectTimeline* pTimeline = pProject->GetTimeline(TimelineTrackTypeTransition);
    if (pTimeline)
    {
        for (size_t e = 0; e < pTimeline->GetExtentCount(); ++e)
        {
            hr = BeginElement(L"transition");
            if (FAILED(hr)) break;

            hr = WriteAttribute(L"extentId", static_cast<DWORD>(pTimeline->GetExtentIdAt(e)));
            if (FAILED(hr)) break;

            hr = EndElement();
            if (FAILED(hr)) break;
        }
    }

    if (SUCCEEDED(hr))
        hr = EndElement();

    return hr;
}

HRESULT SerializationWriter::WriteEffectsElement(MovieProject* pProject)
{
    HRESULT hr = BeginElement(L"effects");
    if (FAILED(hr)) return hr;

    for (size_t m = 0; m < pProject->GetMediaItemCount(); ++m)
    {
        const ProjectMediaItem* pItem = pProject->GetMediaItem(m);
        if (!pItem)
            continue;

        for (int t = 0; t < 6; ++t)
        {
            const ProjectTimeline* pTimeline = pProject->GetTimeline(
                static_cast<TimelineTrackType>(t));
            if (!pTimeline)
                continue;

            for (size_t e = 0; e < pTimeline->GetExtentCount(); ++e)
            {
                DWORD dwExtentId = pTimeline->GetExtentIdAt(e);

                hr = BeginElement(L"effect");
                if (FAILED(hr)) return hr;

                hr = WriteAttribute(L"extentId", dwExtentId);
                if (FAILED(hr)) return hr;

                hr = WriteAttribute(L"mediaId", pItem->GetMediaId());
                if (FAILED(hr)) return hr;

                hr = EndElement();
                if (FAILED(hr)) return hr;
            }
        }
    }

    if (SUCCEEDED(hr))
        hr = EndElement();

    return hr;
}

HRESULT SerializationWriter::WriteTitlesElement(MovieProject* pProject)
{
    HRESULT hr = BeginElement(L"titles");
    if (FAILED(hr)) return hr;

    const ProjectTimeline* pTimeline = pProject->GetTimeline(TimelineTrackTypeTitle);
    if (pTimeline)
    {
        for (size_t e = 0; e < pTimeline->GetExtentCount(); ++e)
        {
            hr = BeginElement(L"title");
            if (FAILED(hr)) break;

            hr = WriteAttribute(L"extentId", static_cast<DWORD>(pTimeline->GetExtentIdAt(e)));
            if (FAILED(hr)) break;

            hr = EndElement();
            if (FAILED(hr)) break;
        }
    }

    if (SUCCEEDED(hr))
        hr = EndElement();

    return hr;
}

HRESULT SerializationWriter::WriteSettingsElement(MovieProject* pProject)
{
    const MovieProjectSettings& settings = pProject->GetSettings();

    HRESULT hr = BeginElement(L"settings");
    if (FAILED(hr)) return hr;

    hr = WriteAttribute(L"outputWidth", static_cast<int>(settings.GetOutputWidth()));
    if (SUCCEEDED(hr))
        hr = WriteAttribute(L"outputHeight", static_cast<int>(settings.GetOutputHeight()));
    if (SUCCEEDED(hr))
        hr = WriteAttribute(L"aspectRatio", settings.GetAspectRatio());
    if (SUCCEEDED(hr))
        hr = WriteAttribute(L"audioBitRate", settings.GetAudioBitRate());
    if (SUCCEEDED(hr))
        hr = WriteAttribute(L"videoBitRate", settings.GetVideoBitRate());
    if (SUCCEEDED(hr))
        hr = WriteAttribute(L"frameRate", settings.GetFrameRate());

    if (SUCCEEDED(hr))
        hr = EndElement();

    return hr;
}

// ============================================================================
// Legacy project writing
// ============================================================================

HRESULT SerializationWriter::SaveProject(LPCWSTR pszFilePath, MovieProject* pProject)
{
    return WriteToFile(pszFilePath, pProject);
}

HRESULT SerializationWriter::WriteProject(LPCWSTR pszFilePath, MovieProject* pProject)
{
    return WriteToFile(pszFilePath, pProject);
}

HRESULT SerializationWriter::WriteMediaItems(IXmlWriter* pWriter, MovieProject* pProject)
{
    if (!pWriter || !pProject)
        return E_POINTER;

    HRESULT hr = pWriter->WriteStartElement(nullptr, L"media", nullptr);
    if (FAILED(hr))
        return hr;

    for (size_t i = 0; i < pProject->GetMediaItemCount(); ++i)
    {
        const ProjectMediaItem* pItem = pProject->GetMediaItem(i);
        if (!pItem)
            continue;

        hr = pWriter->WriteStartElement(nullptr, L"mediaItem", nullptr);
        if (FAILED(hr)) break;

        WCHAR szBuf[64];

        _itow_s(static_cast<int>(pItem->GetMediaId()), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"id", nullptr, szBuf);
        if (FAILED(hr)) break;

        hr = pWriter->WriteAttributeString(nullptr, L"path", nullptr, pItem->GetSourcePath());
        if (FAILED(hr)) break;

        _itow_s(static_cast<int>(pItem->GetMediaType()), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"type", nullptr, szBuf);
        if (FAILED(hr)) break;

        _i64tow_s(pItem->GetDurationHns(), szBuf, _countof(szBuf), 10);
        hr = pWriter->WriteAttributeString(nullptr, L"duration", nullptr, szBuf);
        if (FAILED(hr)) break;

        if (pItem->GetWidth() > 0 && pItem->GetHeight() > 0)
        {
            _itow_s(static_cast<int>(pItem->GetWidth()), szBuf, 10);
            hr = pWriter->WriteAttributeString(nullptr, L"width", nullptr, szBuf);
            if (FAILED(hr)) break;

            _itow_s(static_cast<int>(pItem->GetHeight()), szBuf, 10);
            hr = pWriter->WriteAttributeString(nullptr, L"height", nullptr, szBuf);
            if (FAILED(hr)) break;
        }

        if (pItem->GetFrameRate() > 0)
        {
            _itow_s(pItem->GetFrameRate(), szBuf, 10);
            hr = pWriter->WriteAttributeString(nullptr, L"frameRate", nullptr, szBuf);
            if (FAILED(hr)) break;
        }

        if (SUCCEEDED(hr))
            hr = pWriter->WriteEndElement();
        if (FAILED(hr)) break;
    }

    if (SUCCEEDED(hr))
        hr = pWriter->WriteEndElement();

    return hr;
}

HRESULT SerializationWriter::WriteProperties(IXmlWriter* pWriter, MovieProject* pProject)
{
    if (!pWriter || !pProject)
        return E_POINTER;

    const MovieProjectSettings& settings = pProject->GetSettings();
    WCHAR szBuf[64];

    HRESULT hr = pWriter->WriteStartElement(nullptr, L"settings", nullptr);
    if (FAILED(hr))
        return hr;

    _itow_s(static_cast<int>(settings.GetOutputWidth()), szBuf, 10);
    hr = pWriter->WriteAttributeString(nullptr, L"outputWidth", nullptr, szBuf);
    if (SUCCEEDED(hr))
    {
        _itow_s(static_cast<int>(settings.GetOutputHeight()), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"outputHeight", nullptr, szBuf);
    }

    if (SUCCEEDED(hr))
    {
        _snwprintf_s(szBuf, _countof(szBuf), _TRUNCATE, L"%g", settings.GetAspectRatio());
        hr = pWriter->WriteAttributeString(nullptr, L"aspectRatio", nullptr, szBuf);
    }

    if (SUCCEEDED(hr))
    {
        _itow_s(settings.GetAudioBitRate(), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"audioBitRate", nullptr, szBuf);
    }

    if (SUCCEEDED(hr))
    {
        _itow_s(settings.GetVideoBitRate(), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"videoBitRate", nullptr, szBuf);
    }

    if (SUCCEEDED(hr))
    {
        _itow_s(settings.GetFrameRate(), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"frameRate", nullptr, szBuf);
    }

    if (SUCCEEDED(hr))
        hr = pWriter->WriteEndElement();

    return hr;
}

ATL::CString SerializationWriter::EscapeXmlString(LPCWSTR pszInput)
{
    if (!pszInput)
        return ATL::CString();

    ATL::CString strOutput;

    LPCWSTR pch = pszInput;
    while (*pch)
    {
        switch (*pch)
        {
        case L'&':
            strOutput += L"&amp;";
            break;
        case L'<':
            strOutput += L"&lt;";
            break;
        case L'>':
            strOutput += L"&gt;";
            break;
        case L'"':
            strOutput += L"&quot;";
            break;
        case L'\'':
            strOutput += L"&apos;";
            break;
        default:
            strOutput += *pch;
            break;
        }
        ++pch;
    }

    return strOutput;
}

} // namespace StoryboardManager
