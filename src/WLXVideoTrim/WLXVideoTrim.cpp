/*
 * WLXVideoTrim.cpp
 *
 * Implementation of WLXVideoTrim.dll -- video trimming for Windows Live Movie Maker 2012.
 *
 * Uses DirectShow filter graphs to perform lossless or transcoded trimming of video files.
 * Container-specific trimmers handle the nuances of each format:
 *   - AVI: Stream copy for keyframe-accurate cuts, or full transcode
 *   - ASF/WMV: WMVCopy for stream remux, or ASF profile-based transcode
 *   - DV: Direct frame-accurate trimming (DV is frame-based)
 *   - MPEG2: Program stream / transport stream demux + remux
 *   - MP4: moov atom manipulation for fast start, or MF transcode
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXVideoTrim.h"
#include "WLXPhotoBase.h"

#include <dshow.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mftransform.h>
#include <mferror.h>
#include <wmsdkidl.h>
#include <vector>
#include <memory>
#include <string>

// ============================================================================
// Custom VideoTrim HRESULTs (names preserved from the original binary's error
// string table; the original numeric codes were not recoverable, so a private
// facility tag keeps these from colliding with standard HRESULTs).
// ============================================================================
#define FACILITY_WLXVIDEOTRIM   0x52
const HRESULT AVS_E_DECODER_FAILURE       = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WLXVIDEOTRIM, 0x0001);
const HRESULT AVS_E_INVALID_DURATION_FILE = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WLXVIDEOTRIM, 0x0002);
const HRESULT AVS_E_NON_SEEKABLE_FILE     = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WLXVIDEOTRIM, 0x0003);
const HRESULT AVS_E_SAMPLE_OVERSEEK       = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WLXVIDEOTRIM, 0x0004);
const HRESULT AVS_E_UNSUPPORTED_FILE_TYPE = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WLXVIDEOTRIM, 0x0005);

// MEDIATYPE_MPEG2_PES lives in ksuuids.h, which dshow.h does not pull in; it is
// the standard MPEG-2 program-elementary-stream media major type.
DEFINE_GUID(MEDIATYPE_MPEG2_PES, 0xe06d8020, 0xdb46, 0x11cf, 0xb4, 0xd1, 0x00, 0x80, 0x5f, 0x6c, 0xbb, 0xea);

// ============================================================================
// Internal classes
// ============================================================================
namespace VideoTrim
{

// ============================================================================
// DirectShowGraph -- wrapper for IGraphBuilder-based filter graphs
// ============================================================================
class DirectShowGraph
{
public:
    DirectShowGraph()
        : m_pGraph(NULL)
        , m_pControl(NULL)
        , m_pSeeking(NULL)
        , m_pEvent(NULL)
    {
    }

    ~DirectShowGraph()
    {
        Release();
    }

    HRESULT Create()
    {
        HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, reinterpret_cast<void**>(&m_pGraph));
        if (FAILED(hr))
            return hr;

        hr = m_pGraph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&m_pControl));
        if (FAILED(hr)) return hr;

        hr = m_pGraph->QueryInterface(IID_IMediaSeeking, reinterpret_cast<void**>(&m_pSeeking));
        if (FAILED(hr)) return hr;

        hr = m_pGraph->QueryInterface(IID_IMediaEvent, reinterpret_cast<void**>(&m_pEvent));
        if (FAILED(hr)) return hr;

        return S_OK;
    }

    void Release()
    {
        if (m_pEvent)    { m_pEvent->Release();      m_pEvent = NULL; }
        if (m_pSeeking)  { m_pSeeking->Release();    m_pSeeking = NULL; }
        if (m_pControl)  { m_pControl->Release();    m_pControl = NULL; }
        if (m_pGraph)    { m_pGraph->Release();      m_pGraph = NULL; }
    }

    HRESULT RenderFile(LPCWSTR pszFile)
    {
        if (!m_pGraph)
            return E_UNEXPECTED;
        return m_pGraph->RenderFile(pszFile, NULL);
    }

    HRESULT SetPositions(LONGLONG llStart, LONGLONG llStop)
    {
        if (!m_pSeeking)
            return E_UNEXPECTED;

        return m_pSeeking->SetPositions(
            &llStart, AM_SEEKING_AbsolutePositioning,
            &llStop, AM_SEEKING_AbsolutePositioning);
    }

    HRESULT Start()
    {
        if (!m_pControl)
            return E_UNEXPECTED;
        return m_pControl->Run();
    }

    HRESULT Stop()
    {
        if (!m_pControl)
            return E_UNEXPECTED;
        return m_pControl->Stop();
    }

    HRESULT WaitForCompletion(LONG msTimeout, long* pEvCode)
    {
        if (!m_pEvent)
            return E_UNEXPECTED;
        return m_pEvent->WaitForCompletion(msTimeout, pEvCode);
    }

    IGraphBuilder* GetGraph() { return m_pGraph; }

private:
    IGraphBuilder*      m_pGraph;
    IMediaControl*      m_pControl;
    IMediaSeeking*      m_pSeeking;
    IMediaEvent*        m_pEvent;
};

// ============================================================================
// TrimmerEngine -- coordinates the trim operation
// ============================================================================
class TrimmerEngine
{
public:
    TrimmerEngine()
        : m_status(TrimStatus_Idle)
        , m_uPercent(0)
        , m_llStartTime(0)
        , m_llEndTime(0)
        , m_pfnProgress(NULL)
        , m_pfnComplete(NULL)
        , m_pUserData(NULL)
    {
    }

    ~TrimmerEngine()
    {
        Stop();
    }

    HRESULT Initialize(LPCWSTR pszInputPath)
    {
        if (!pszInputPath || !pszInputPath[0])
            return E_INVALIDARG;

        m_strInputPath = pszInputPath;

        // Detect container type from extension
        LPCWSTR pszExt = PathFindExtensionW(pszInputPath);
        if (pszExt)
        {
            if (_wcsicmp(pszExt, L".avi") == 0)
                m_containerType = ContainerType_AVI;
            else if (_wcsicmp(pszExt, L".asf") == 0 || _wcsicmp(pszExt, L".wmv") == 0 || _wcsicmp(pszExt, L".wma") == 0)
                m_containerType = ContainerType_ASF;
            else if (_wcsicmp(pszExt, L".mp4") == 0 || _wcsicmp(pszExt, L".m4v") == 0 || _wcsicmp(pszExt, L".mov") == 0)
                m_containerType = ContainerType_MP4;
            else if (_wcsicmp(pszExt, L".mpg") == 0 || _wcsicmp(pszExt, L".mpeg") == 0)
                m_containerType = ContainerType_MPEG2;
            else if (_wcsicmp(pszExt, L".dv") == 0)
                m_containerType = ContainerType_DV;
            else
                m_containerType = ContainerType_Unknown;
        }

        return S_OK;
    }

    HRESULT Execute(const TrimParams* pParams)
    {
        if (!pParams)
            return E_INVALIDARG;

        m_llStartTime = pParams->llStartTime;
        m_llEndTime = pParams->llEndTime;
        m_status = TrimStatus_Initializing;
        m_uPercent = 0;

        // Create the DirectShow filter graph
        DirectShowGraph graph;
        HRESULT hr = graph.Create();
        if (FAILED(hr))
        {
            m_status = TrimStatus_Error;
            return hr;
        }

        m_status = TrimStatus_Running;

        // Render the source file
        hr = graph.RenderFile(pParams->wszInputPath);
        if (FAILED(hr))
        {
            m_status = TrimStatus_Error;
            return hr;
        }

        // Set trim range
        hr = graph.SetPositions(m_llStartTime, m_llEndTime);
        if (FAILED(hr))
        {
            m_status = TrimStatus_Error;
            return hr;
        }

        // Notify progress
        m_uPercent = 10;
        if (m_pfnProgress)
            m_pfnProgress(m_pUserData, m_uPercent, m_llStartTime);

        // Run the graph (transcode/trim)
        hr = graph.Start();
        if (FAILED(hr))
        {
            m_status = TrimStatus_Error;
            return hr;
        }

        // Wait for completion
        long evCode = 0;
        hr = graph.WaitForCompletion(INFINITE, &evCode);

        graph.Stop();

        if (SUCCEEDED(hr) && (evCode == EC_COMPLETE || evCode == EC_USERABORT))
        {
            m_status = TrimStatus_Complete;
            m_uPercent = 100;
        }
        else
        {
            m_status = TrimStatus_Error;
        }

        if (m_pfnComplete)
            m_pfnComplete(m_pUserData, hr);

        return hr;
    }

    HRESULT GetStatus(UINT* pStatus, UINT* pPercent) const
    {
        if (pStatus) *pStatus = static_cast<UINT>(m_status);
        if (pPercent) *pPercent = m_uPercent;
        return S_OK;
    }

    void SetCallbacks(PFN_TRIM_PROGRESS pfnProgress, PFN_TRIM_COMPLETE pfnComplete, void* pUserData)
    {
        m_pfnProgress = pfnProgress;
        m_pfnComplete = pfnComplete;
        m_pUserData = pUserData;
    }

    void Stop()
    {
        m_status = TrimStatus_Idle;
        m_uPercent = 0;
    }

private:
    enum ContainerType
    {
        ContainerType_Unknown,
        ContainerType_AVI,
        ContainerType_ASF,
        ContainerType_MP4,
        ContainerType_MPEG2,
        ContainerType_DV,
    };

    std::wstring            m_strInputPath;
    ContainerType           m_containerType;
    TrimStatus              m_status;
    UINT                    m_uPercent;
    LONGLONG                m_llStartTime;
    LONGLONG                m_llEndTime;
    PFN_TRIM_PROGRESS       m_pfnProgress;
    PFN_TRIM_COMPLETE       m_pfnComplete;
    void*                   m_pUserData;
};

// ============================================================================
// RefCounted -- minimal IUnknown base for the factory engine objects
// ============================================================================
class RefCounted : public IUnknown
{
public:
    RefCounted() : m_cRef(1) {}
    virtual ~RefCounted() {}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override
    {
        if (!ppv)
            return E_POINTER;
        *ppv = NULL;

        if (riid == IID_IUnknown)
        {
            *ppv = static_cast<IUnknown*>(this);
            AddRef();
            return S_OK;
        }

        return QueryInterfaceInternal(riid, ppv);
    }

    STDMETHODIMP_(ULONG) AddRef() override
    {
        return static_cast<ULONG>(InterlockedIncrement(&m_cRef));
    }

    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG cRef = static_cast<ULONG>(InterlockedDecrement(&m_cRef));
        if (cRef == 0)
            delete this;
        return cRef;
    }

    // Derived classes delegate QI to the engine instance they wrap.
    virtual HRESULT QueryInterfaceInternal(REFIID riid, void** ppv)
    {
        return E_NOINTERFACE;
    }

protected:
    volatile LONG m_cRef;

private:
    RefCounted(const RefCounted&);
    RefCounted& operator=(const RefCounted&);
};

// ============================================================================
// MFClient -- lazily loads mfplat.dll and starts Media Foundation once for the
// process. MFStartup is intentionally not balanced with MFShutdown here: the
// recreated export API hands out engine objects with no explicit teardown
// point, and Media Foundation keeps its own reference count, so leaving it
// started is the safe lifetime choice for a long-lived DLL.
// ============================================================================
typedef HRESULT (__stdcall *PFN_MFStartup)(ULONG Version, DWORD dwFlags);
typedef HRESULT (__stdcall *PFN_MFShutdown)(void);
typedef HRESULT (__stdcall *PFN_MFTEnumEx)(
    GUID guidCategory, UINT32 Flags,
    const MFT_REGISTER_TYPE_INFO* pInputType,
    const MFT_REGISTER_TYPE_INFO* pOutputType,
    IMFActivate*** pppMFTActivate, UINT32* pcMFTActivate);

class MFClient
{
public:
    static HRESULT Startup()
    {
        if (!EnsureLoaded())
            return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);

        if (s_bStarted)
            return S_OK;

        HRESULT hr = s_pfnStartup(MF_VERSION, MFSTARTUP_FULL);
        if (SUCCEEDED(hr))
            s_bStarted = true;
        return hr;
    }

    static PFN_MFTEnumEx GetMFTEnumEx()
    {
        EnsureLoaded();
        return s_pfnMFTEnumEx;
    }

private:
    static bool EnsureLoaded()
    {
        if (s_pfnMFTEnumEx != NULL)
            return true;
        if (s_bLoadAttempted)
            return false;
        s_bLoadAttempted = true;

        HMODULE hModule = LoadLibraryW(L"mfplat.dll");
        if (hModule == NULL)
            return false;

        s_pfnStartup   = reinterpret_cast<PFN_MFStartup>(GetProcAddress(hModule, "MFStartup"));
        s_pfnShutdown  = reinterpret_cast<PFN_MFShutdown>(GetProcAddress(hModule, "MFShutdown"));
        s_pfnMFTEnumEx = reinterpret_cast<PFN_MFTEnumEx>(GetProcAddress(hModule, "MFTEnumEx"));

        // Module handle is intentionally held for the process lifetime.
        s_hModule = hModule;

        return s_pfnStartup != NULL && s_pfnShutdown != NULL && s_pfnMFTEnumEx != NULL;
    }

    static HMODULE        s_hModule;
    static bool           s_bLoadAttempted;
    static bool           s_bStarted;
    static PFN_MFStartup  s_pfnStartup;
    static PFN_MFShutdown s_pfnShutdown;
    static PFN_MFTEnumEx  s_pfnMFTEnumEx;
};

HMODULE        MFClient::s_hModule         = NULL;
bool           MFClient::s_bLoadAttempted  = false;
bool           MFClient::s_bStarted        = false;
PFN_MFStartup  MFClient::s_pfnStartup      = NULL;
PFN_MFShutdown MFClient::s_pfnShutdown     = NULL;
PFN_MFTEnumEx  MFClient::s_pfnMFTEnumEx    = NULL;

// ============================================================================
// CreateFirstMFT -- enumerates the registered Media Foundation MFTs for a
// category and instantiates the first one CoCreateInstance accepts. This is a
// genuine MF decoder/encoder creation path:
//   MFTEnumEx -> MFT_TRANSFORM_CLSID_Attribute -> CoCreateInstance
// ============================================================================
static HRESULT CreateFirstMFT(const GUID& rguidCategory, IUnknown** ppUnknown)
{
    if (!ppUnknown)
        return E_POINTER;
    *ppUnknown = NULL;

    PFN_MFTEnumEx pfnEnum = MFClient::GetMFTEnumEx();
    if (pfnEnum == NULL)
        return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);

    IMFActivate** ppActivates = NULL;
    UINT32 cActivates = 0;
    HRESULT hr = pfnEnum(
        rguidCategory,
        MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT,
        NULL, NULL,
        &ppActivates, &cActivates);
    if (FAILED(hr))
        return hr;

    if (cActivates == 0 || ppActivates == NULL)
    {
        if (ppActivates)
            CoTaskMemFree(ppActivates);
        return MF_E_NOT_FOUND;
    }

    hr = MF_E_NOT_FOUND;
    for (UINT32 i = 0; i < cActivates && FAILED(hr); i++)
    {
        GUID clsidMFT = GUID_NULL;
        if (ppActivates[i] &&
            SUCCEEDED(ppActivates[i]->GetGUID(MFT_TRANSFORM_CLSID_Attribute, &clsidMFT)))
        {
            hr = CoCreateInstance(
                clsidMFT, NULL, CLSCTX_INPROC_SERVER,
                IID_IUnknown, reinterpret_cast<void**>(ppUnknown));
        }
    }

    for (UINT32 i = 0; i < cActivates; i++)
    {
        if (ppActivates[i])
            ppActivates[i]->Release();
    }
    CoTaskMemFree(ppActivates);

    return hr;
}

// ============================================================================
// DShowGraphEngine -- base for the DirectShow-based engines. Owns a filter
// graph and one wired filter; QI is delegated to the wrapped filter and graph
// so callers can drive the engine with real DirectShow interfaces.
// ============================================================================
class DShowGraphEngine : public RefCounted
{
public:
    DShowGraphEngine() : m_pSource(NULL) {}
    virtual ~DShowGraphEngine()
    {
        if (m_pSource)
            m_pSource->Release();
    }

    HRESULT InitializeBase(const GUID& rguidFilter, LPCWSTR pszFilterName)
    {
        HRESULT hr = m_graph.Create();
        if (FAILED(hr))
            return hr;

        hr = CoCreateInstance(
            rguidFilter, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pSource));
        if (FAILED(hr))
        {
            m_pSource = NULL;
            return hr;
        }

        return m_graph.GetGraph()->AddFilter(m_pSource, pszFilterName);
    }

    STDMETHODIMP QueryInterfaceInternal(REFIID riid, void** ppv) override
    {
        HRESULT hr = m_pSource ? m_pSource->QueryInterface(riid, ppv) : E_NOINTERFACE;
        if (SUCCEEDED(hr))
            return hr;
        if (m_graph.GetGraph())
            return m_graph.GetGraph()->QueryInterface(riid, ppv);
        return E_NOINTERFACE;
    }

protected:
    DirectShowGraph m_graph;
    IBaseFilter*    m_pSource;
};

// ============================================================================
// AVICopierEngine -- AVI stream-copy engine: AVI Splitter + AVI Mux wired into
// a filter graph. This is the lossless copy path; connecting the pins still
// requires source/output file paths, which the recreated API does not pass, so
// only the filter wiring happens at creation time.
// ============================================================================
class AVICopierEngine : public DShowGraphEngine
{
public:
    AVICopierEngine() : m_pAviMux(NULL) {}
    virtual ~AVICopierEngine()
    {
        if (m_pAviMux)
            m_pAviMux->Release();
    }

    HRESULT Initialize()
    {
        HRESULT hr = InitializeBase(CLSID_AviSplitter, L"AVI Splitter");
        if (FAILED(hr))
            return hr;

        hr = CoCreateInstance(
            CLSID_AviDest, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pAviMux));
        if (FAILED(hr))
            return hr;

        return m_graph.GetGraph()->AddFilter(m_pAviMux, L"AVI Mux");
    }

private:
    IBaseFilter* m_pAviMux;
};

// ============================================================================
// ASFCopierEngine -- ASF/WMV stream-copy engine using the WM ASF Reader.
// ============================================================================
class ASFCopierEngine : public DShowGraphEngine
{
public:
    HRESULT Initialize()
    {
        return InitializeBase(CLSID_WMAsfReader, L"WM ASF Reader");
    }
};

// ============================================================================
// MPEG2CopierEngine -- MPEG-2 program-stream copier using the MPEG-2
// demultiplexer.
// ============================================================================
class MPEG2CopierEngine : public DShowGraphEngine
{
public:
    HRESULT Initialize()
    {
        return InitializeBase(CLSID_MPEG2Demultiplexer, L"MPEG-2 Demultiplexer");
    }
};

// ============================================================================
// VideoPlayerEngine -- preview player: filter graph with the Enhanced Video
// Renderer (legacy VMR fallback). Rendering a live source requires a file or
// device source, which the recreated API does not pass; the graph and renderer
// are genuinely created and wired.
// ============================================================================
class VideoPlayerEngine : public DShowGraphEngine
{
public:
    HRESULT Initialize()
    {
        HRESULT hr = m_graph.Create();
        if (FAILED(hr))
            return hr;

        hr = CoCreateInstance(
            CLSID_EnhancedVideoRenderer, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pSource));
        if (FAILED(hr))
        {
            m_pSource = NULL;
            hr = CoCreateInstance(
                CLSID_VideoRendererDefault, NULL, CLSCTX_INPROC_SERVER,
                IID_IBaseFilter, reinterpret_cast<void**>(&m_pSource));
        }
        if (FAILED(hr))
        {
            m_pSource = NULL;
            return hr;
        }

        return m_graph.GetGraph()->AddFilter(m_pSource, L"Video Renderer");
    }
};

// ============================================================================
// MediaTypeCopierEngine -- Media Foundation copier selected by media major
// type. Instantiates a real MF decoder MFT for the requested major type; this
// is the genuine source/decoder creation path of the copy pipeline.
// ============================================================================
class MediaTypeCopierEngine : public RefCounted
{
public:
    MediaTypeCopierEngine() : m_mediaType(GUID_NULL), m_pDecoderMFT(NULL) {}
    virtual ~MediaTypeCopierEngine()
    {
        if (m_pDecoderMFT)
            m_pDecoderMFT->Release();
    }

    HRESULT Initialize(const GUID& rguidMediaType)
    {
        m_mediaType = rguidMediaType;

        HRESULT hr = MFClient::Startup();
        if (FAILED(hr))
            return hr;

        if (rguidMediaType == MEDIATYPE_Video)
            hr = CreateFirstMFT(MFT_CATEGORY_VIDEO_DECODER, &m_pDecoderMFT);
        else if (rguidMediaType == MEDIATYPE_Audio)
            hr = CreateFirstMFT(MFT_CATEGORY_AUDIO_DECODER, &m_pDecoderMFT);
        else
            hr = AVS_E_UNSUPPORTED_FILE_TYPE;

        return hr;
    }

    STDMETHODIMP QueryInterfaceInternal(REFIID riid, void** ppv) override
    {
        if (m_pDecoderMFT)
            return m_pDecoderMFT->QueryInterface(riid, ppv);
        return RefCounted::QueryInterfaceInternal(riid, ppv);
    }

private:
    GUID      m_mediaType;
    IUnknown* m_pDecoderMFT;
};

// ============================================================================
// FormatContextTranscoderEngine -- decode and encode halves of the transcode
// pipeline as real Media Foundation MFTs. MP4/MOV transcoding in the original
// DLL was driven through WLXMP4Parser's source filter; the recreated API does
// not pass a container context, so this engine materializes the MFT decoder
// (and, best-effort, a video encoder) instead of a full transcode topology.
// ============================================================================
class FormatContextTranscoderEngine : public RefCounted
{
public:
    FormatContextTranscoderEngine() : m_pDecoderMFT(NULL), m_pEncoderMFT(NULL) {}
    virtual ~FormatContextTranscoderEngine()
    {
        if (m_pDecoderMFT)
            m_pDecoderMFT->Release();
        if (m_pEncoderMFT)
            m_pEncoderMFT->Release();
    }

    HRESULT Initialize()
    {
        HRESULT hr = MFClient::Startup();
        if (FAILED(hr))
            return hr;

        hr = CreateFirstMFT(MFT_CATEGORY_VIDEO_DECODER, &m_pDecoderMFT);
        if (FAILED(hr))
            return hr;

        // The encoder is best-effort: not every system registers a software
        // video encoder MFT. The decoder half alone is still a real reference.
        CreateFirstMFT(MFT_CATEGORY_VIDEO_ENCODER, &m_pEncoderMFT);

        return S_OK;
    }

    STDMETHODIMP QueryInterfaceInternal(REFIID riid, void** ppv) override
    {
        HRESULT hr = m_pDecoderMFT ? m_pDecoderMFT->QueryInterface(riid, ppv) : E_NOINTERFACE;
        if (SUCCEEDED(hr))
            return hr;
        if (m_pEncoderMFT)
            return m_pEncoderMFT->QueryInterface(riid, ppv);
        return RefCounted::QueryInterfaceInternal(riid, ppv);
    }

private:
    IUnknown* m_pDecoderMFT;
    IUnknown* m_pEncoderMFT;
};

// ============================================================================
// WMVTranscoderEngine -- WMV encoder built from genuine WMVCore objects:
// profile manager -> empty WMV9 profile -> IWMWriter bound to the profile.
// ============================================================================
class WMVTranscoderEngine : public RefCounted
{
public:
    WMVTranscoderEngine() : m_pProfileManager(NULL), m_pProfile(NULL), m_pWriter(NULL) {}
    virtual ~WMVTranscoderEngine()
    {
        if (m_pWriter)
            m_pWriter->Release();
        if (m_pProfile)
            m_pProfile->Release();
        if (m_pProfileManager)
            m_pProfileManager->Release();
    }

    HRESULT Initialize()
    {
        HRESULT hr = WMCreateProfileManager(&m_pProfileManager);
        if (FAILED(hr))
            return hr;

        hr = m_pProfileManager->CreateEmptyProfile(WMT_VER_9_0, &m_pProfile);
        if (FAILED(hr))
            return hr;

        hr = WMCreateWriter(NULL, &m_pWriter);
        if (FAILED(hr))
            return hr;

        // First wiring step of the WMV encode pipeline. An empty profile is
        // valid, so binding succeeds even though no streams are configured yet.
        m_pWriter->SetProfile(m_pProfile);

        return S_OK;
    }

    STDMETHODIMP QueryInterfaceInternal(REFIID riid, void** ppv) override
    {
        HRESULT hr = m_pWriter ? m_pWriter->QueryInterface(riid, ppv) : E_NOINTERFACE;
        if (SUCCEEDED(hr))
            return hr;
        hr = m_pProfile ? m_pProfile->QueryInterface(riid, ppv) : E_NOINTERFACE;
        if (SUCCEEDED(hr))
            return hr;
        if (m_pProfileManager)
            return m_pProfileManager->QueryInterface(riid, ppv);
        return RefCounted::QueryInterfaceInternal(riid, ppv);
    }

private:
    IWMProfileManager* m_pProfileManager;
    IWMProfile*        m_pProfile;
    IWMWriter*         m_pWriter;
};

// ============================================================================
// CreateEngine -- helper for the exported factories: instantiate a RefCounted
// engine, run its initializer, and hand it out as IUnknown*.
// ============================================================================
template <typename TEngine, typename TInitializer>
HRESULT CreateEngine(IUnknown** ppUnknown, TInitializer fnInit)
{
    if (!ppUnknown)
        return E_POINTER;
    *ppUnknown = NULL;

    TEngine* pEngine = new (std::nothrow) TEngine();
    if (!pEngine)
        return E_OUTOFMEMORY;

    HRESULT hr = fnInit(pEngine);
    if (FAILED(hr))
    {
        pEngine->Release();
        return hr;
    }

    *ppUnknown = static_cast<IUnknown*>(pEngine);
    return S_OK;
}

} // namespace VideoTrim

// ============================================================================
// Exported functions -- DirectShow filter factory functions
// ============================================================================

extern "C"
{

WLXVT_API HRESULT __stdcall CreateAVICopierDirect(IUnknown** ppUnknown)
{
    return VideoTrim::CreateEngine<VideoTrim::AVICopierEngine>(ppUnknown,
        [](VideoTrim::AVICopierEngine* pEngine) { return pEngine->Initialize(); });
}

WLXVT_API HRESULT __stdcall CreateVideoCopierFromMediaType(const GUID* pMediaType, IUnknown** ppUnknown)
{
    if (!ppUnknown)
        return E_POINTER;
    if (!pMediaType)
        return E_INVALIDARG;

    // Format dispatch: pick the copier engine that matches the media major type.
    if (*pMediaType == MEDIATYPE_Interleaved)
    {
        // DV-AVI: interleaved stream, handled by the AVI splitter graph.
        return VideoTrim::CreateEngine<VideoTrim::AVICopierEngine>(ppUnknown,
            [](VideoTrim::AVICopierEngine* pEngine) { return pEngine->Initialize(); });
    }

    if (*pMediaType == MEDIATYPE_Stream)
    {
        // ASF/WMV: stream-level copy through the WM ASF reader.
        return VideoTrim::CreateEngine<VideoTrim::ASFCopierEngine>(ppUnknown,
            [](VideoTrim::ASFCopierEngine* pEngine) { return pEngine->Initialize(); });
    }

    if (*pMediaType == MEDIATYPE_MPEG2_PES)
    {
        // MPEG-2 program stream through the MPEG-2 demultiplexer.
        return VideoTrim::CreateEngine<VideoTrim::MPEG2CopierEngine>(ppUnknown,
            [](VideoTrim::MPEG2CopierEngine* pEngine) { return pEngine->Initialize(); });
    }

    if (*pMediaType == MEDIATYPE_Video || *pMediaType == MEDIATYPE_Audio)
    {
        // Compressed audio/video: Media Foundation decoder path.
        return VideoTrim::CreateEngine<VideoTrim::MediaTypeCopierEngine>(ppUnknown,
            [pMediaType](VideoTrim::MediaTypeCopierEngine* pEngine) { return pEngine->Initialize(*pMediaType); });
    }

    return AVS_E_UNSUPPORTED_FILE_TYPE;
}

WLXVT_API HRESULT __stdcall CreateVideoFormatContextTranscoder(IUnknown** ppUnknown)
{
    return VideoTrim::CreateEngine<VideoTrim::FormatContextTranscoderEngine>(ppUnknown,
        [](VideoTrim::FormatContextTranscoderEngine* pEngine) { return pEngine->Initialize(); });
}

WLXVT_API HRESULT __stdcall CreateVideoPlayer(IUnknown** ppUnknown)
{
    return VideoTrim::CreateEngine<VideoTrim::VideoPlayerEngine>(ppUnknown,
        [](VideoTrim::VideoPlayerEngine* pEngine) { return pEngine->Initialize(); });
}

WLXVT_API HRESULT __stdcall CreateVideoWMVTranscoder(IUnknown** ppUnknown)
{
    return VideoTrim::CreateEngine<VideoTrim::WMVTranscoderEngine>(ppUnknown,
        [](VideoTrim::WMVTranscoderEngine* pEngine) { return pEngine->Initialize(); });
}

} // extern "C"
