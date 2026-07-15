// SAXProfileBuilder.cpp - SAX-based XML profile parser implementation

#include "pch.h"
#include <xmllite.h>
#include "SAXProfileBuilder.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

SAXProfileBuilder::SAXProfileBuilder()
    : m_cRef(1)
    , m_pCurrentProfile(nullptr)
    , m_fInVideoSection(false)
    , m_fInAudioSection(false)
    , m_fInConstraints(false)
{
}

SAXProfileBuilder::~SAXProfileBuilder()
{
}

// ============================================================================
// IUnknown
// ============================================================================

STDMETHODIMP SAXProfileBuilder::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    *ppvObject = nullptr;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<SAXProfileBuilder*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) SAXProfileBuilder::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) SAXProfileBuilder::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
    return cRef;
}

// ============================================================================
// Profile loading
// ============================================================================

HRESULT SAXProfileBuilder::LoadProfile(LPCWSTR pszFilePath, EncodeProfile* pProfile)
{
    if (!pszFilePath || !pProfile)
        return E_POINTER;

    if (!PathFileExistsW(pszFilePath))
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_READ, &spStream);
    if (FAILED(hr))
        return hr;

    return ParseFromStream(spStream, pProfile);
}

HRESULT SAXProfileBuilder::LoadProfilesFromDirectory(
    LPCWSTR pszDirectory,
    ATL::CAtlArray<EncodeProfile>& profiles)
{
    if (!pszDirectory)
        return E_POINTER;

    profiles.RemoveAll();

    ATL::CString strSearch;
    strSearch.Format(L"%s\\*.xml", pszDirectory);

    WIN32_FIND_DATAW findData = {};
    HANDLE hFind = FindFirstFileW(strSearch.GetString(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return S_OK;

    do
    {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        ATL::CString strFilePath;
        strFilePath.Format(L"%s\\%s", pszDirectory, findData.cFileName);

        EncodeProfile profile;
        HRESULT hr = LoadProfile(strFilePath.GetString(), &profile);
        if (SUCCEEDED(hr))
            profiles.Add(profile);

    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return S_OK;
}

HRESULT SAXProfileBuilder::LoadDefaultProfiles()
{
    // Default profiles are built-in and don't require XML files.
    // This method loads any user-customized profiles from the profiles directory.
    return S_OK;
}

// ============================================================================
// Profile saving
// ============================================================================

HRESULT SAXProfileBuilder::SaveProfile(LPCWSTR pszFilePath, const EncodeProfile& profile)
{
    if (!pszFilePath)
        return E_POINTER;

    CComPtr<IXmlWriter> spWriter;
    HRESULT hr = CreateXmlWriter(pszFilePath, &spWriter);
    if (FAILED(hr))
        return hr;

    return WriteProfileXml(spWriter, profile);
}

// ============================================================================
// Direct XML parsing
// ============================================================================

HRESULT SAXProfileBuilder::ParseFromString(LPCWSTR pszXml, DWORD cchXml, EncodeProfile* pProfile)
{
    if (!pszXml || !pProfile || cchXml == 0)
        return E_POINTER;

    m_pCurrentProfile = pProfile;
    m_fInVideoSection = false;
    m_fInAudioSection = false;
    m_fInConstraints = false;

    CComPtr<IStream> spStream;
    HRESULT hr = CreateStreamOnHGlobal(nullptr, TRUE, &spStream);
    if (FAILED(hr))
        return hr;

    hr = spStream->Write(pszXml, cchXml * sizeof(WCHAR), nullptr);
    if (FAILED(hr))
        return hr;

    LARGE_INTEGER liZero = {};
    spStream->Seek(liZero, STREAM_SEEK_SET, nullptr);

    hr = ParseFromStream(spStream, pProfile);

    m_pCurrentProfile = nullptr;
    return hr;
}

HRESULT SAXProfileBuilder::ParseFromStream(IStream* pStream, EncodeProfile* pProfile)
{
    if (!pStream || !pProfile)
        return E_POINTER;

    m_pCurrentProfile = pProfile;
    m_fInVideoSection = false;
    m_fInAudioSection = false;
    m_fInConstraints = false;

    CComPtr<IXmlReader> spReader;
    HRESULT hr = CreateXmlReader(&spReader);
    if (FAILED(hr))
        return hr;

    hr = spReader->SetInput(pStream);
    if (FAILED(hr))
        return hr;

    XmlNodeType nodeType;
    while (SUCCEEDED(spReader->Read(&nodeType)))
    {
        switch (nodeType)
        {
        case XmlNodeType_Element:
            hr = ProcessStartElement(spReader);
            break;
        case XmlNodeType_EndElement:
            hr = ProcessEndElement(spReader);
            break;
        case XmlNodeType_Text:
        {
            const WCHAR* pValue = nullptr;
            if (SUCCEEDED(spReader->GetValue(&pValue, nullptr)))
                m_strCurrentValue = pValue ? pValue : L"";
            break;
        }
        default:
            break;
        }

        if (FAILED(hr))
            break;
    }

    m_pCurrentProfile = nullptr;
    return hr;
}

// ============================================================================
// Profile directory
// ============================================================================

ATL::CString SAXProfileBuilder::GetProfileDirectory()
{
    ATL::CString strPath;
    TCHAR szAppData[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, szAppData)))
    {
        strPath.Format(L"%s\\Microsoft\\WL\\MovieMaker\\Profiles", szAppData);
    }
    return strPath;
}

HRESULT SAXProfileBuilder::EnsureProfileDirectory()
{
    ATL::CString strDir = GetProfileDirectory();
    if (strDir.IsEmpty())
        return E_FAIL;

    if (!PathFileExistsW(strDir.GetString()))
    {
        HRESULT hr = SHCreateDirectoryExW(nullptr, strDir.GetString(), nullptr);
        if (FAILED(hr) && hr != ERROR_ALREADY_EXISTS)
            return hr;
    }

    return S_OK;
}

// ============================================================================
// XmlLite helpers
// ============================================================================

HRESULT SAXProfileBuilder::CreateXmlReader(IXmlReader** ppReader)
{
    return ::CreateXmlReader(IID_PPV_ARGS(ppReader), nullptr);
}

HRESULT SAXProfileBuilder::CreateXmlWriter(LPCWSTR pszFilePath, IXmlWriter** ppWriter)
{
    if (!ppWriter)
        return E_POINTER;

    CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_WRITE | STGM_CREATE, &spStream);
    if (FAILED(hr))
        return hr;

    return ::CreateXmlWriter(IID_PPV_ARGS(ppWriter), nullptr);
}

HRESULT SAXProfileBuilder::ProcessStartElement(IXmlReader* pReader)
{
    if (!pReader || !m_pCurrentProfile)
        return S_OK;

    const WCHAR* pName = nullptr;
    HRESULT hr = pReader->GetLocalName(&pName, nullptr);
    if (FAILED(hr) || !pName)
        return S_OK;

    m_strCurrentElement = pName;
    m_strCurrentValue.Empty();

    if (m_strCurrentElement.CompareNoCase(L"video") == 0)
    {
        m_fInVideoSection = true;
        m_fInAudioSection = false;
    }
    else if (m_strCurrentElement.CompareNoCase(L"audio") == 0)
    {
        m_fInAudioSection = true;
        m_fInVideoSection = false;
    }
    else if (m_strCurrentElement.CompareNoCase(L"constraints") == 0)
    {
        m_fInConstraints = true;
    }
    else if (m_strCurrentElement.CompareNoCase(L"profile") == 0)
    {
        hr = ProcessAttributes(pReader);
    }

    return S_OK;
}

HRESULT SAXProfileBuilder::ProcessEndElement(IXmlReader* pReader)
{
    if (!pReader)
        return S_OK;

    const WCHAR* pName = nullptr;
    HRESULT hr = pReader->GetLocalName(&pName, nullptr);
    if (FAILED(hr) || !pName)
        return S_OK;

    return ProcessEndElement(ATL::CString(pName));
}

HRESULT SAXProfileBuilder::ProcessEndElement(const ATL::CString& strElement)
{
    if (strElement.CompareNoCase(L"video") == 0)
        m_fInVideoSection = false;
    else if (strElement.CompareNoCase(L"audio") == 0)
        m_fInAudioSection = false;
    else if (strElement.CompareNoCase(L"constraints") == 0)
        m_fInConstraints = false;

    return S_OK;
}

HRESULT SAXProfileBuilder::ProcessAttributes(IXmlReader* pReader)
{
    if (!pReader)
        return S_OK;

    const WCHAR* pLocalName = nullptr;
    const WCHAR* pValue = nullptr;

    while (SUCCEEDED(pReader->MoveToNextAttribute()))
    {
        if (SUCCEEDED(pReader->GetLocalName(&pLocalName, nullptr)) &&
            SUCCEEDED(pReader->GetValue(&pValue, nullptr)) &&
            pLocalName && pValue)
        {
            OnProfileAttribute(ATL::CString(pLocalName), ATL::CString(pValue));
        }
    }

    pReader->MoveToElement();
    return S_OK;
}

// ============================================================================
// Attribute value parsing
// ============================================================================

UINT SAXProfileBuilder::ParseUInt(const ATL::CString& strValue, UINT uDefault)
{
    UINT uResult = uDefault;
    if (!strValue.IsEmpty())
        uResult = static_cast<UINT>(_ttoi(strValue.GetString()));
    return uResult;
}

DWORD SAXProfileBuilder::ParseDWord(const ATL::CString& strValue, DWORD dwDefault)
{
    DWORD dwResult = dwDefault;
    if (!strValue.IsEmpty())
        dwResult = static_cast<DWORD>(_ttoi(strValue.GetString()));
    return dwResult;
}

double SAXProfileBuilder::ParseDouble(const ATL::CString& strValue, double dblDefault)
{
    double dblResult = dblDefault;
    if (!strValue.IsEmpty())
        dblResult = _ttof(strValue.GetString());
    return dblResult;
}

bool SAXProfileBuilder::ParseBool(const ATL::CString& strValue, bool fDefault)
{
    if (strValue.IsEmpty())
        return fDefault;
    return strValue.CompareNoCase(L"true") == 0 || strValue.CompareNoCase(L"1") == 0;
}

VideoCodec SAXProfileBuilder::ParseVideoCodec(const ATL::CString& strValue)
{
    if (strValue.CompareNoCase(L"h264") == 0 || strValue.CompareNoCase(L"avc") == 0)
        return VideoCodecH264;
    if (strValue.CompareNoCase(L"wmv9") == 0 || strValue.CompareNoCase(L"vc1") == 0)
        return VideoCodecWMV9;
    if (strValue.CompareNoCase(L"mpeg4") == 0)
        return VideoCodecMPEG4;
    if (strValue.CompareNoCase(L"h265") == 0 || strValue.CompareNoCase(L"hevc") == 0)
        return VideoCodecH265;
    return VideoCodecUnknown;
}

AudioCodec SAXProfileBuilder::ParseAudioCodec(const ATL::CString& strValue)
{
    if (strValue.CompareNoCase(L"aac") == 0)
        return AudioCodecAAC;
    if (strValue.CompareNoCase(L"wma") == 0)
        return AudioCodecWMA;
    if (strValue.CompareNoCase(L"mp3") == 0)
        return AudioCodecMP3;
    if (strValue.CompareNoCase(L"pcm") == 0)
        return AudioCodecPCM;
    return AudioCodecUnknown;
}

// ============================================================================
// Element handlers
// ============================================================================

HRESULT SAXProfileBuilder::HandleProfileElement(IXmlReader* pReader)
{
    return ProcessAttributes(pReader);
}

HRESULT SAXProfileBuilder::HandleVideoElement(IXmlReader* pReader)
{
    if (!pReader || !m_pCurrentProfile)
        return S_OK;

    const WCHAR* pLocalName = nullptr;
    const WCHAR* pValue = nullptr;

    EncodeVideoParams& video = m_pCurrentProfile->GetVideoParams();

    while (SUCCEEDED(pReader->MoveToNextAttribute()))
    {
        if (SUCCEEDED(pReader->GetLocalName(&pLocalName, nullptr)) &&
            SUCCEEDED(pReader->GetValue(&pValue, nullptr)) &&
            pLocalName && pValue)
        {
            ATL::CString strName(pLocalName);
            ATL::CString strValue(pValue);

            if (strName.CompareNoCase(L"width") == 0)
                video.uWidth = ParseUInt(strValue, video.uWidth);
            else if (strName.CompareNoCase(L"height") == 0)
                video.uHeight = ParseUInt(strValue, video.uHeight);
            else if (strName.CompareNoCase(L"framerate") == 0)
                video.dblFrameRate = ParseDouble(strValue, video.dblFrameRate);
            else if (strName.CompareNoCase(L"bitrate") == 0)
                video.dwBitRate = ParseDWord(strValue, video.dwBitRate);
            else if (strName.CompareNoCase(L"peakbitrate") == 0)
                video.dwPeakBitRate = ParseDWord(strValue, video.dwPeakBitRate);
            else if (strName.CompareNoCase(L"codec") == 0)
                video.codec = ParseVideoCodec(strValue);
            else if (strName.CompareNoCase(L"profile") == 0)
                video.uProfile = ParseUInt(strValue, video.uProfile);
            else if (strName.CompareNoCase(L"level") == 0)
                video.uLevel = ParseUInt(strValue, video.uLevel);
            else if (strName.CompareNoCase(L"twopass") == 0)
                video.fTwoPass = ParseBool(strValue, video.fTwoPass);
            else if (strName.CompareNoCase(L"hwaccel") == 0)
                video.fHardwareAcceleration = ParseBool(strValue, video.fHardwareAcceleration);
        }
    }

    pReader->MoveToElement();
    return S_OK;
}

HRESULT SAXProfileBuilder::HandleAudioElement(IXmlReader* pReader)
{
    if (!pReader || !m_pCurrentProfile)
        return S_OK;

    const WCHAR* pLocalName = nullptr;
    const WCHAR* pValue = nullptr;

    EncodeAudioParams& audio = m_pCurrentProfile->GetAudioParams();

    while (SUCCEEDED(pReader->MoveToNextAttribute()))
    {
        if (SUCCEEDED(pReader->GetLocalName(&pLocalName, nullptr)) &&
            SUCCEEDED(pReader->GetValue(&pValue, nullptr)) &&
            pLocalName && pValue)
        {
            ATL::CString strName(pLocalName);
            ATL::CString strValue(pValue);

            if (strName.CompareNoCase(L"samplerate") == 0)
                audio.dwSampleRate = ParseDWord(strValue, audio.dwSampleRate);
            else if (strName.CompareNoCase(L"channels") == 0)
                audio.dwChannels = ParseDWord(strValue, audio.dwChannels);
            else if (strName.CompareNoCase(L"bitspersample") == 0)
                audio.dwBitsPerSample = ParseDWord(strValue, audio.dwBitsPerSample);
            else if (strName.CompareNoCase(L"bitrate") == 0)
                audio.dwBitRate = ParseDWord(strValue, audio.dwBitRate);
            else if (strName.CompareNoCase(L"codec") == 0)
                audio.codec = ParseAudioCodec(strValue);
        }
    }

    pReader->MoveToElement();
    return S_OK;
}

HRESULT SAXProfileBuilder::HandleConstraintsElement(IXmlReader* pReader)
{
    if (!pReader || !m_pCurrentProfile)
        return S_OK;

    const WCHAR* pLocalName = nullptr;
    const WCHAR* pValue = nullptr;

    while (SUCCEEDED(pReader->MoveToNextAttribute()))
    {
        if (SUCCEEDED(pReader->GetLocalName(&pLocalName, nullptr)) &&
            SUCCEEDED(pReader->GetValue(&pValue, nullptr)) &&
            pLocalName && pValue)
        {
            ATL::CString strName(pLocalName);
            ATL::CString strValue(pValue);

            if (strName.CompareNoCase(L"maxduration") == 0)
            {
                LONGLONG llMax = _ttoi64(strValue.GetString());
                m_pCurrentProfile->SetMaxDurationHns(llMax);
            }
            else if (strName.CompareNoCase(L"maxfilesize") == 0)
            {
                DWORD dwMax = ParseDWord(strValue, 0);
                m_pCurrentProfile->SetMaxFileSizeBytes(dwMax);
            }
        }
    }

    pReader->MoveToElement();
    return S_OK;
}

// ============================================================================
// Profile attribute handling
// ============================================================================

HRESULT SAXProfileBuilder::OnProfileAttribute(const ATL::CString& strName, const ATL::CString& strValue)
{
    if (!m_pCurrentProfile)
        return S_OK;

    if (strName.CompareNoCase(L"id") == 0)
    {
        int id = _ttoi(strValue.GetString());
        m_pCurrentProfile->SetProfileId(static_cast<EncodeProfileId>(id));
    }
    else if (strName.CompareNoCase(L"name") == 0)
    {
        m_pCurrentProfile->SetProfileName(strValue.GetString());
    }

    return S_OK;
}

HRESULT SAXProfileBuilder::OnVideoAttribute(const ATL::CString& strName, const ATL::CString& strValue)
{
    UNREFERENCED_PARAMETER(strName);
    UNREFERENCED_PARAMETER(strValue);
    return S_OK;
}

HRESULT SAXProfileBuilder::OnAudioAttribute(const ATL::CString& strName, const ATL::CString& strValue)
{
    UNREFERENCED_PARAMETER(strName);
    UNREFERENCED_PARAMETER(strValue);
    return S_OK;
}

HRESULT SAXProfileBuilder::OnConstraintsAttribute(const ATL::CString& strName, const ATL::CString& strValue)
{
    UNREFERENCED_PARAMETER(strName);
    UNREFERENCED_PARAMETER(strValue);
    return S_OK;
}

// ============================================================================
// XML writing
// ============================================================================

HRESULT SAXProfileBuilder::WriteProfileXml(IXmlWriter* pWriter, const EncodeProfile& profile)
{
    if (!pWriter)
        return E_POINTER;

    pWriter->SetProperty(XmlWriterProperty_ImplicitFragmentEscape, TRUE);

    HRESULT hr = pWriter->WriteStartDocument(XmlStandalone_Omit);
    if (FAILED(hr))
        return hr;

    hr = pWriter->WriteStartElement(nullptr, L"profile", nullptr);
    if (FAILED(hr))
        return hr;

    pWriter->WriteAttributeString(nullptr, L"id", nullptr,
        ATL::CStringW(std::to_wstring(profile.GetProfileId()).c_str()));
    pWriter->WriteAttributeString(nullptr, L"name", nullptr, profile.GetProfileName());

    hr = WriteVideoParams(pWriter, profile.GetVideoParams());
    if (FAILED(hr))
        return hr;

    hr = WriteAudioParams(pWriter, profile.GetAudioParams());
    if (FAILED(hr))
        return hr;

    if (profile.GetMaxDurationHns() > 0 || profile.GetMaxFileSizeBytes() > 0)
    {
        hr = pWriter->WriteStartElement(nullptr, L"constraints", nullptr);
        if (FAILED(hr))
            return hr;

        if (profile.GetMaxDurationHns() > 0)
        {
            pWriter->WriteAttributeString(nullptr, L"maxduration", nullptr,
                ATL::CStringW(std::to_wstring(profile.GetMaxDurationHns()).c_str()));
        }
        if (profile.GetMaxFileSizeBytes() > 0)
        {
            pWriter->WriteAttributeString(nullptr, L"maxfilesize", nullptr,
                ATL::CStringW(std::to_wstring(profile.GetMaxFileSizeBytes()).c_str()));
        }

        hr = pWriter->WriteEndElement();
        if (FAILED(hr))
            return hr;
    }

    hr = pWriter->WriteEndElement();
    if (FAILED(hr))
        return hr;

    hr = pWriter->WriteEndDocument();
    return hr;
}

HRESULT SAXProfileBuilder::WriteVideoParams(IXmlWriter* pWriter, const EncodeVideoParams& params)
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr = pWriter->WriteStartElement(nullptr, L"video", nullptr);
    if (FAILED(hr))
        return hr;

    pWriter->WriteAttributeString(nullptr, L"width", nullptr,
        ATL::CStringW(std::to_wstring(params.uWidth).c_str()));
    pWriter->WriteAttributeString(nullptr, L"height", nullptr,
        ATL::CStringW(std::to_wstring(params.uHeight).c_str()));
    pWriter->WriteAttributeString(nullptr, L"framerate", nullptr,
        ATL::CStringW(std::to_wstring(static_cast<UINT>(params.dblFrameRate)).c_str()));
    pWriter->WriteAttributeString(nullptr, L"bitrate", nullptr,
        ATL::CStringW(std::to_wstring(params.dwBitRate).c_str()));
    pWriter->WriteAttributeString(nullptr, L"peakbitrate", nullptr,
        ATL::CStringW(std::to_wstring(params.dwPeakBitRate).c_str()));

    LPCWSTR pszCodec = L"h264";
    switch (params.codec)
    {
    case VideoCodecH264:    pszCodec = L"h264";    break;
    case VideoCodecWMV9:    pszCodec = L"wmv9";    break;
    case VideoCodecMPEG4:   pszCodec = L"mpeg4";   break;
    case VideoCodecH265:    pszCodec = L"h265";     break;
    default:                pszCodec = L"h264";     break;
    }
    pWriter->WriteAttributeString(nullptr, L"codec", nullptr, pszCodec);

    pWriter->WriteAttributeString(nullptr, L"profile", nullptr,
        ATL::CStringW(std::to_wstring(params.uProfile).c_str()));
    pWriter->WriteAttributeString(nullptr, L"level", nullptr,
        ATL::CStringW(std::to_wstring(params.uLevel).c_str()));
    pWriter->WriteAttributeString(nullptr, L"twopass", nullptr,
        params.fTwoPass ? L"true" : L"false");
    pWriter->WriteAttributeString(nullptr, L"hwaccel", nullptr,
        params.fHardwareAcceleration ? L"true" : L"false");

    return pWriter->WriteEndElement();
}

HRESULT SAXProfileBuilder::WriteAudioParams(IXmlWriter* pWriter, const EncodeAudioParams& params)
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr = pWriter->WriteStartElement(nullptr, L"audio", nullptr);
    if (FAILED(hr))
        return hr;

    pWriter->WriteAttributeString(nullptr, L"samplerate", nullptr,
        ATL::CStringW(std::to_wstring(params.dwSampleRate).c_str()));
    pWriter->WriteAttributeString(nullptr, L"channels", nullptr,
        ATL::CStringW(std::to_wstring(params.dwChannels).c_str()));
    pWriter->WriteAttributeString(nullptr, L"bitspersample", nullptr,
        ATL::CStringW(std::to_wstring(params.dwBitsPerSample).c_str()));
    pWriter->WriteAttributeString(nullptr, L"bitrate", nullptr,
        ATL::CStringW(std::to_wstring(params.dwBitRate).c_str()));

    LPCWSTR pszCodec = L"aac";
    switch (params.codec)
    {
    case AudioCodecAAC:     pszCodec = L"aac";     break;
    case AudioCodecWMA:     pszCodec = L"wma";     break;
    case AudioCodecMP3:     pszCodec = L"mp3";     break;
    case AudioCodecPCM:     pszCodec = L"pcm";     break;
    default:                pszCodec = L"aac";     break;
    }
    pWriter->WriteAttributeString(nullptr, L"codec", nullptr, pszCodec);

    return pWriter->WriteEndElement();
}

} // namespace HMRAVSource
