/*
 * WLMFDS.cpp
 *
 * Implementation of WLMFDS.dll -- Media Foundation / DirectShow bridge.
 *
 * Provides the compatibility layer between legacy DirectShow filter graphs
 * and the modern Media Foundation pipeline. Enables Movie Maker to:
 *   - Use DirectShow source filters for legacy codec support
 *   - Route DShow audio/video through MF transforms
 *   - Present video via EVR with hardware acceleration
 *   - Schedule real-time playback via AVRT (multimedia class scheduler)
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLMFDS.h"
#include "WLXPhotoBase.h"

#include <dshow.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <evr.h>
#include <vfwmsgs.h>
#include <shlwapi.h>
#include <vector>
#include <deque>
#include <memory>

// ----------------------------------------------------------------------------
// MF_SAMPLE_FLAG_* were historically public macros; this SDK no longer exposes
// them, so keep local definitions for the sample flag mapping below.
// ----------------------------------------------------------------------------
#ifndef MF_SAMPLE_FLAG_CLEAN_POINT
#define MF_SAMPLE_FLAG_CLEAN_POINT 0x1
#endif
#ifndef MF_SAMPLE_FLAG_DISCONTINUITY
#define MF_SAMPLE_FLAG_DISCONTINUITY 0x2
#endif
#ifndef MF_SAMPLE_FLAG_PREROLL
#define MF_SAMPLE_FLAG_PREROLL 0x4
#endif

// ============================================================================
// Internal classes
// ============================================================================
namespace MFDS
{

// ----------------------------------------------------------------------------
// Minimal COM smart pointer (internal; mirrors the CComPtr usage model)
// ----------------------------------------------------------------------------
template <typename T>
class ComPtr
{
public:
    ComPtr() : m_p(NULL) {}
    explicit ComPtr(T* p) : m_p(p) {}
    ComPtr(const ComPtr& other) : m_p(other.m_p)
    {
        if (m_p) m_p->AddRef();
    }
    ~ComPtr() { Release(); }

    ComPtr& operator=(T* p)
    {
        if (m_p != p)
        {
            Release();
            m_p = p;
        }
        return *this;
    }
    ComPtr& operator=(const ComPtr& other)
    {
        if (m_p != other.m_p)
        {
            T* p = other.m_p;
            if (p) p->AddRef();
            Release();
            m_p = p;
        }
        return *this;
    }

    T* operator->() const { return m_p; }
    T* Get() const { return m_p; }
    operator T*() const { return m_p; }

    T* Detach() { T* p = m_p; m_p = NULL; return p; }
    void Attach(T* p) { Release(); m_p = p; }
    void Release() { if (m_p) { m_p->Release(); m_p = NULL; } }

    T** operator&() { return &m_p; }
    T** ReleaseAndGetAddressOf() { Release(); return &m_p; }

private:
    T* m_p;
};

// ----------------------------------------------------------------------------
// Critical-section helper
// ----------------------------------------------------------------------------
class AutoLock
{
public:
    explicit AutoLock(CRITICAL_SECTION* pcs) : m_pcs(pcs) { EnterCriticalSection(m_pcs); }
    ~AutoLock() { LeaveCriticalSection(m_pcs); }
private:
    CRITICAL_SECTION* m_pcs;
};

// ----------------------------------------------------------------------------
// DirectShow AM_MEDIA_TYPE helpers
// ----------------------------------------------------------------------------
void FreeAMMediaType(AM_MEDIA_TYPE* pmt)
{
    if (!pmt) return;

    if (pmt->cbFormat != 0)
    {
        CoTaskMemFree(pmt->pbFormat);
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    if (pmt->pUnk != NULL)
    {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }
}

void DeleteAMMediaType(AM_MEDIA_TYPE* pmt)
{
    if (!pmt) return;
    FreeAMMediaType(pmt);
    CoTaskMemFree(pmt);
}

HRESULT CopyAMMediaType(const AM_MEDIA_TYPE* pSrc, AM_MEDIA_TYPE** ppDst)
{
    if (!pSrc || !ppDst) return E_POINTER;
    *ppDst = NULL;

    AM_MEDIA_TYPE* pDst = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (!pDst) return E_OUTOFMEMORY;

    *pDst = *pSrc;
    pDst->pUnk = NULL;
    pDst->pbFormat = NULL;

    if (pSrc->cbFormat != 0)
    {
        pDst->pbFormat = (BYTE*)CoTaskMemAlloc(pSrc->cbFormat);
        if (!pDst->pbFormat)
        {
            CoTaskMemFree(pDst);
            return E_OUTOFMEMORY;
        }
        CopyMemory(pDst->pbFormat, pSrc->pbFormat, pSrc->cbFormat);
    }
    if (pSrc->pUnk != NULL)
    {
        pDst->pUnk = pSrc->pUnk;
        pDst->pUnk->AddRef();
    }

    *ppDst = pDst;
    return S_OK;
}

// Fill a caller-provided AM_MEDIA_TYPE with a deep copy of pSrc. The caller
// owns the resulting format block and must free it (DeleteAMMediaType).
HRESULT FillAMMediaType(const AM_MEDIA_TYPE* pSrc, AM_MEDIA_TYPE* pmt)
{
    if (!pSrc || !pmt) return E_POINTER;

    *pmt = *pSrc;
    pmt->pUnk = NULL;
    pmt->pbFormat = NULL;

    if (pSrc->cbFormat != 0)
    {
        pmt->pbFormat = (BYTE*)CoTaskMemAlloc(pSrc->cbFormat);
        if (!pmt->pbFormat) return E_OUTOFMEMORY;
        CopyMemory(pmt->pbFormat, pSrc->pbFormat, pSrc->cbFormat);
    }
    if (pSrc->pUnk != NULL)
    {
        pmt->pUnk = pSrc->pUnk;
        pmt->pUnk->AddRef();
    }
    return S_OK;
}

// RAII for an owned AM_MEDIA_TYPE* (freed via DeleteAMMediaType)
struct AMMediaTypeGuard
{
    AM_MEDIA_TYPE* p;

    AMMediaTypeGuard() : p(NULL) {}
    ~AMMediaTypeGuard() { if (p) DeleteAMMediaType(p); }

    AM_MEDIA_TYPE* operator->() { return p; }
    AM_MEDIA_TYPE** operator&() { return &p; }
    void Reset(AM_MEDIA_TYPE* np) { if (p) DeleteAMMediaType(p); p = np; }
};

// Convert a negotiated DirectShow media type to an MF media type. This is the
// primary DS->MF format conversion used by the media source descriptors.
HRESULT CreateMFTypeFromAMType(const AM_MEDIA_TYPE* pAMType, IMFMediaType** ppMFType)
{
    if (!pAMType || !ppMFType) return E_POINTER;
    *ppMFType = NULL;
    return MFCreateMediaTypeFromRepresentation(
        AM_MEDIA_TYPE_REPRESENTATION, const_cast<AM_MEDIA_TYPE*>(pAMType), ppMFType);
}

// Build a DirectShow VIDEOINFOHEADER-backed media type for the MF->DS video
// push path (the EVR input pin accepts these).
HRESULT BuildAMVideoType(UINT32 cx, UINT32 cy, const GUID& subtype, AM_MEDIA_TYPE** ppAMType)
{
    if (!ppAMType) return E_POINTER;
    *ppAMType = NULL;

    DWORD fourcc = subtype.Data1; // FourCC subtypes (NV12/YUY2/...)
    WORD biBitCount = 0;
    DWORD biSizeImage = 0;
    if (subtype == MEDIASUBTYPE_NV12)
    {
        biBitCount = 12;
        biSizeImage = cx * cy * 3 / 2;
    }
    else if (subtype == MEDIASUBTYPE_YUY2)
    {
        biBitCount = 16;
        biSizeImage = cx * cy * 2;
    }
    else
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    AM_MEDIA_TYPE* pmt = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (!pmt) return E_OUTOFMEMORY;
    ZeroMemory(pmt, sizeof(AM_MEDIA_TYPE));

    pmt->majortype = MEDIATYPE_Video;
    pmt->subtype = subtype;
    pmt->bFixedSizeSamples = TRUE;
    pmt->bTemporalCompression = FALSE;
    pmt->lSampleSize = biSizeImage;
    pmt->formattype = FORMAT_VideoInfo;
    pmt->cbFormat = sizeof(VIDEOINFOHEADER);
    pmt->pbFormat = (BYTE*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
    if (!pmt->pbFormat)
    {
        CoTaskMemFree(pmt);
        return E_OUTOFMEMORY;
    }
    ZeroMemory(pmt->pbFormat, sizeof(VIDEOINFOHEADER));

    VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)pmt->pbFormat;
    pvih->rcSource.right = (LONG)cx;
    pvih->rcSource.bottom = (LONG)cy;
    pvih->rcTarget = pvih->rcSource;
    pvih->AvgTimePerFrame = 333667; // ~29.97 fps default frame duration
    pvih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pvih->bmiHeader.biWidth = (LONG)cx;
    pvih->bmiHeader.biHeight = (LONG)cy;
    pvih->bmiHeader.biPlanes = 1;
    pvih->bmiHeader.biBitCount = biBitCount;
    pvih->bmiHeader.biCompression = fourcc;
    pvih->bmiHeader.biSizeImage = biSizeImage;

    *ppAMType = pmt;
    return S_OK;
}

// Convert a DirectShow media sample to an MF sample by copying the payload
// into an aligned MF memory buffer (MF and DShow both use 100ns units for
// timestamps, so no timebase conversion is required).
HRESULT ConvertDShowSampleToMF(IMediaSample* pDSSample, IMFSample** ppSample)
{
    if (!pDSSample || !ppSample) return E_POINTER;
    *ppSample = NULL;

    BYTE* pData = NULL;
    HRESULT hr = pDSSample->GetPointer(&pData);
    if (FAILED(hr)) return hr;

    long cbLen = pDSSample->GetActualDataLength();
    if (cbLen < 0) cbLen = 0;

    ComPtr<IMFSample> spSample;
    hr = MFCreateSample(&spSample);
    if (FAILED(hr)) return hr;

    ComPtr<IMFMediaBuffer> spBuffer;
    hr = MFCreateAlignedMemoryBuffer((DWORD)cbLen, 32, spBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    BYTE* pDest = NULL;
    DWORD cbMax = 0;
    DWORD cbCur = 0;
    hr = spBuffer->Lock(&pDest, &cbMax, &cbCur);
    if (FAILED(hr)) return hr;
    if (cbLen > 0 && pData != NULL)
        CopyMemory(pDest, pData, cbLen);
    hr = spBuffer->SetCurrentLength((DWORD)cbLen);
    spBuffer->Unlock();
    if (FAILED(hr)) return hr;

    hr = spSample->AddBuffer(spBuffer);
    if (FAILED(hr)) return hr;

    REFERENCE_TIME tStart = 0;
    REFERENCE_TIME tEnd = 0;
    if (pDSSample->GetTime(&tStart, &tEnd) == S_OK)
    {
        spSample->SetSampleTime(tStart);
        spSample->SetSampleDuration(tEnd - tStart);
    }

    DWORD dwFlags = 0;
    if (pDSSample->IsSyncPoint() == S_OK)      dwFlags |= MF_SAMPLE_FLAG_CLEAN_POINT;
    if (pDSSample->IsDiscontinuity() == S_OK)  dwFlags |= MF_SAMPLE_FLAG_DISCONTINUITY;
    if (pDSSample->IsPreroll() == S_OK)        dwFlags |= MF_SAMPLE_FLAG_PREROLL;
    spSample->SetSampleFlags(dwFlags);

    *ppSample = spSample.Detach();
    return S_OK;
}

// ----------------------------------------------------------------------------
// Forward declarations (circular ownership)
// ----------------------------------------------------------------------------
class CDShowMediaSource;
class CDShowMediaStream;

// ----------------------------------------------------------------------------
// CMediaSampleAdapter -- lightweight IMediaSample implementation that wraps an
// MF sample payload for delivery into a DirectShow renderer (MF->DS push).
// ----------------------------------------------------------------------------
class CMediaSampleAdapter : public IMediaSample2
{
public:
    CMediaSampleAdapter()
        : m_cRef(1)
        , m_bSyncPoint(TRUE)
        , m_bPreroll(FALSE)
        , m_bDiscontinuity(FALSE)
        , m_bHasTime(FALSE)
        , m_tStart(0)
        , m_tEnd(0)
    {
        ZeroMemory(&m_mt, sizeof(m_mt));
    }

    ~CMediaSampleAdapter()
    {
        FreeAMMediaType(&m_mt);
    }

    // Public setup helpers (not part of IMediaSample)
    HRESULT SetData(const BYTE* pData, long cbData)
    {
        if (cbData < 0) return E_INVALIDARG;
        if (cbData > 0 && pData == NULL) return E_POINTER;
        m_data.assign(pData, pData + cbData);
        return S_OK;
    }

    HRESULT SetTimeStamps(REFERENCE_TIME tStart, REFERENCE_TIME tEnd)
    {
        m_tStart = tStart;
        m_tEnd = tEnd;
        m_bHasTime = TRUE;
        return S_OK;
    }

    HRESULT SetSampleFlags(BOOL bSyncPoint, BOOL bPreroll, BOOL bDiscontinuity)
    {
        m_bSyncPoint = bSyncPoint;
        m_bPreroll = bPreroll;
        m_bDiscontinuity = bDiscontinuity;
        return S_OK;
    }

    HRESULT SetMediaTypeCopy(const AM_MEDIA_TYPE* pmt)
    {
        if (!pmt) return E_POINTER;
        FreeAMMediaType(&m_mt);
        AM_MEDIA_TYPE* pCopy = NULL;
        HRESULT hr = CopyAMMediaType(pmt, &pCopy);
        if (FAILED(hr)) return hr;
        m_mt = *pCopy;
        CoTaskMemFree(pCopy);
        return S_OK;
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IMediaSample || riid == IID_IMediaSample2)
        {
            *ppv = static_cast<IMediaSample2*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG ulRef = InterlockedDecrement(&m_cRef);
        if (ulRef == 0) delete this;
        return ulRef;
    }

    // IMediaSample
    STDMETHODIMP GetPointer(BYTE** ppBuffer)
    {
        if (!ppBuffer) return E_POINTER;
        *ppBuffer = m_data.empty() ? NULL : &m_data[0];
        return S_OK;
    }

    STDMETHODIMP_(long) GetSize() { return (long)m_data.size(); }

    STDMETHODIMP GetTime(REFERENCE_TIME* pTimeStart, REFERENCE_TIME* pTimeEnd)
    {
        if (!pTimeStart || !pTimeEnd) return E_POINTER;
        if (!m_bHasTime) return VFW_E_SAMPLE_TIME_NOT_SET;
        *pTimeStart = m_tStart;
        *pTimeEnd = m_tEnd;
        return S_OK;
    }

    STDMETHODIMP SetTime(REFERENCE_TIME* pTimeStart, REFERENCE_TIME* pTimeEnd)
    {
        if (pTimeStart) m_tStart = *pTimeStart;
        if (pTimeEnd)   m_tEnd = *pTimeEnd;
        m_bHasTime = TRUE;
        return S_OK;
    }

    STDMETHODIMP IsSyncPoint() { return m_bSyncPoint ? S_OK : S_FALSE; }
    STDMETHODIMP SetSyncPoint(BOOL bIsSyncPoint) { m_bSyncPoint = bIsSyncPoint; return S_OK; }
    STDMETHODIMP IsPreroll() { return m_bPreroll ? S_OK : S_FALSE; }
    STDMETHODIMP SetPreroll(BOOL bIsPreroll) { m_bPreroll = bIsPreroll; return S_OK; }

    STDMETHODIMP_(long) GetActualDataLength() { return (long)m_data.size(); }

    STDMETHODIMP SetActualDataLength(long lLen)
    {
        if (lLen < 0 || (size_t)lLen > m_data.size()) return E_INVALIDARG;
        m_data.resize((size_t)lLen);
        return S_OK;
    }

    STDMETHODIMP GetMediaType(AM_MEDIA_TYPE** ppMediaType)
    {
        if (!ppMediaType) return E_POINTER;
        *ppMediaType = NULL;
        if (m_mt.majortype == GUID_NULL && m_mt.cbFormat == 0 && m_mt.pbFormat == NULL)
            return S_FALSE;
        return CopyAMMediaType(&m_mt, ppMediaType);
    }

    STDMETHODIMP SetMediaType(AM_MEDIA_TYPE* pMediaType)
    {
        return SetMediaTypeCopy(pMediaType);
    }

    STDMETHODIMP IsDiscontinuity() { return m_bDiscontinuity ? S_OK : S_FALSE; }
    STDMETHODIMP SetDiscontinuity(BOOL bDiscontinuity) { m_bDiscontinuity = bDiscontinuity; return S_OK; }

    STDMETHODIMP GetMediaTime(LONGLONG* pTimeStart, LONGLONG* pTimeEnd)
    {
        if (!pTimeStart || !pTimeEnd) return E_POINTER;
        if (!m_bHasTime) return VFW_E_MEDIA_TIME_NOT_SET;
        *pTimeStart = m_tStart;
        *pTimeEnd = m_tEnd;
        return S_OK;
    }

    STDMETHODIMP SetMediaTime(LONGLONG* pTimeStart, LONGLONG* pTimeEnd)
    {
        return SetTime(pTimeStart, pTimeEnd);
    }

    // IMediaSample2
    STDMETHODIMP GetProperties(DWORD cbProperties, BYTE* pbProperties)
    {
        if (!pbProperties) return E_POINTER;
        if (cbProperties < sizeof(AM_SAMPLE2_PROPERTIES)) return E_INVALIDARG;

        AM_SAMPLE2_PROPERTIES* pProps = (AM_SAMPLE2_PROPERTIES*)pbProperties;
        ZeroMemory(pProps, sizeof(AM_SAMPLE2_PROPERTIES));
        pProps->cbData = sizeof(AM_SAMPLE2_PROPERTIES);
        pProps->dwTypeSpecificFlags = 0;
        pProps->dwSampleFlags = 0;
        if (m_bSyncPoint)     pProps->dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;
        if (m_bPreroll)       pProps->dwSampleFlags |= AM_SAMPLE_PREROLL;
        if (m_bDiscontinuity) pProps->dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
        if (m_bHasTime)       pProps->dwSampleFlags |= AM_SAMPLE_TIMEVALID;
        pProps->lActual = (LONG)m_data.size();
        if (m_bHasTime)
        {
            pProps->tStart = m_tStart;
            pProps->tStop = m_tEnd;
        }
        pProps->dwStreamId = AM_STREAM_MEDIA;
        pProps->pMediaType = NULL;
        pProps->pbBuffer = m_data.empty() ? NULL : &m_data[0];
        pProps->cbBuffer = (LONG)m_data.size();
        return S_OK;
    }

    STDMETHODIMP SetProperties(DWORD cbProperties, const BYTE* pbProperties)
    {
        if (!pbProperties) return E_POINTER;
        if (cbProperties < sizeof(AM_SAMPLE2_PROPERTIES)) return E_INVALIDARG;

        const AM_SAMPLE2_PROPERTIES* pProps = (const AM_SAMPLE2_PROPERTIES*)pbProperties;
        m_bSyncPoint     = (pProps->dwSampleFlags & AM_SAMPLE_SPLICEPOINT) ? TRUE : FALSE;
        m_bPreroll       = (pProps->dwSampleFlags & AM_SAMPLE_PREROLL) ? TRUE : FALSE;
        m_bDiscontinuity = (pProps->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY) ? TRUE : FALSE;
        if (pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID)
        {
            m_tStart = pProps->tStart;
            m_tEnd = pProps->tStop;
            m_bHasTime = TRUE;
        }
        if (pProps->cbBuffer > 0 && pProps->pbBuffer)
            m_data.assign(pProps->pbBuffer, pProps->pbBuffer + pProps->cbBuffer);
        return S_OK;
    }

private:
    LONG                m_cRef;
    std::vector<BYTE>   m_data;
    BOOL                m_bSyncPoint;
    BOOL                m_bPreroll;
    BOOL                m_bDiscontinuity;
    BOOL                m_bHasTime;
    REFERENCE_TIME      m_tStart;
    REFERENCE_TIME      m_tEnd;
    AM_MEDIA_TYPE       m_mt;
};

// ----------------------------------------------------------------------------
// CMFVideoOutputPin -- DirectShow output pin that pushes MF video samples into
// a connected renderer (the MF->DS half of the bridge, e.g. EVR).
// ----------------------------------------------------------------------------
class CMFVideoOutputPin : public IPin
{
public:
    CMFVideoOutputPin()
        : m_cRef(1)
        , m_bConnected(FALSE)
        , m_bSampleTypeSent(FALSE)
    {
        ZeroMemory(&m_mt, sizeof(m_mt));
        wcscpy_s(m_achName, L"MFBridgeOut");
    }

    virtual ~CMFVideoOutputPin()
    {
        if (m_mt.pbFormat != NULL)
            FreeAMMediaType(&m_mt);
    }

    // Bridge helper: connect this output pin to an existing renderer input pin.
    HRESULT ConnectToInput(IPin* pInputPin, const AM_MEDIA_TYPE* pmt)
    {
        if (!pInputPin || !pmt) return E_POINTER;
        if (m_pConnected) return VFW_E_ALREADY_CONNECTED;

        HRESULT hr = pInputPin->ReceiveConnection(this, pmt);
        if (FAILED(hr)) return hr;

        AM_MEDIA_TYPE* pCopy = NULL;
        hr = CopyAMMediaType(pmt, &pCopy);
        if (FAILED(hr))
        {
            pInputPin->Disconnect();
            return hr;
        }
        if (m_mt.pbFormat != NULL)
            FreeAMMediaType(&m_mt);
        m_mt = *pCopy;
        CoTaskMemFree(pCopy);

        m_pConnected = pInputPin;
        m_bConnected = TRUE;
        return S_OK;
    }

    // Bridge helper: convert an MF sample and push it downstream.
    HRESULT DeliverSample(IMFSample* pMFSample)
    {
        if (!pMFSample) return E_INVALIDARG;
        if (!m_bConnected || !m_pConnected) return VFW_E_NOT_CONNECTED;

        LONGLONG tStart = 0;
        LONGLONG tDuration = 0;
        if (FAILED(pMFSample->GetSampleTime(&tStart))) tStart = 0;
        if (FAILED(pMFSample->GetSampleDuration(&tDuration))) tDuration = 0;

        CMediaSampleAdapter* pAdapter = new CMediaSampleAdapter();
        if (!pAdapter) return E_OUTOFMEMORY;
        ComPtr<CMediaSampleAdapter> spAdapter(pAdapter);

        DWORD dwFlags = 0;
        pMFSample->GetSampleFlags(&dwFlags);
        spAdapter->SetSampleFlags(
            (dwFlags & MF_SAMPLE_FLAG_CLEAN_POINT) ? TRUE : FALSE,
            (dwFlags & MF_SAMPLE_FLAG_PREROLL) ? TRUE : FALSE,
            (dwFlags & MF_SAMPLE_FLAG_DISCONTINUITY) ? TRUE : FALSE);
        spAdapter->SetTimeStamps(tStart, tStart + tDuration);

        ComPtr<IMFMediaBuffer> spBuffer;
        HRESULT hr = pMFSample->GetBufferByIndex(0, spBuffer.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return hr;

        BYTE* pData = NULL;
        DWORD cbMax = 0;
        DWORD cbCur = 0;
        hr = spBuffer->Lock(&pData, &cbMax, &cbCur);
        if (FAILED(hr)) return hr;
        hr = spAdapter->SetData(pData, (long)cbCur);
        spBuffer->Unlock();
        if (FAILED(hr)) return hr;

        // The EVR's input pin needs the media type on the first delivered
        // sample; subsequent samples inherit it from the connection.
        if (!m_bSampleTypeSent && m_mt.majortype != GUID_NULL)
        {
            spAdapter->SetMediaTypeCopy(&m_mt);
            m_bSampleTypeSent = TRUE;
        }

        ComPtr<IMemInputPin> spMemInputPin;
        HRESULT hrRecv = m_pConnected->QueryInterface(IID_IMemInputPin,
            reinterpret_cast<void**>(&spMemInputPin));
        if (FAILED(hrRecv)) return hrRecv;
        return spMemInputPin->Receive(spAdapter);
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IPin)
        {
            *ppv = static_cast<IPin*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG ulRef = InterlockedDecrement(&m_cRef);
        if (ulRef == 0) delete this;
        return ulRef;
    }

    // IPin
    STDMETHODIMP Connect(IPin* pReceivePin, const AM_MEDIA_TYPE* pmt)
    {
        if (!pReceivePin) return E_POINTER;
        if (m_pConnected) return VFW_E_ALREADY_CONNECTED;

        const AM_MEDIA_TYPE* pmtToUse = pmt ? pmt : &m_mt;
        if (!pmtToUse || pmtToUse->majortype == GUID_NULL) return E_INVALIDARG;

        HRESULT hr = pReceivePin->ReceiveConnection(this, pmtToUse);
        if (FAILED(hr)) return hr;

        AM_MEDIA_TYPE* pCopy = NULL;
        hr = CopyAMMediaType(pmtToUse, &pCopy);
        if (FAILED(hr)) return hr;
        if (m_mt.pbFormat != NULL)
            FreeAMMediaType(&m_mt);
        m_mt = *pCopy;
        CoTaskMemFree(pCopy);

        m_pConnected = pReceivePin;
        m_bConnected = TRUE;
        return S_OK;
    }

    STDMETHODIMP ReceiveConnection(IPin* /*pConnector*/, const AM_MEDIA_TYPE* /*pmt*/)
    {
        // Only input pins accept connections.
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    STDMETHODIMP Disconnect()
    {
        if (!m_bConnected || !m_pConnected) return VFW_E_NOT_CONNECTED;
        m_pConnected.Release();
        m_bConnected = FALSE;
        return S_OK;
    }

    STDMETHODIMP ConnectedTo(IPin** ppPin)
    {
        if (!ppPin) return E_POINTER;
        if (!m_bConnected || !m_pConnected) return VFW_E_NOT_CONNECTED;
        *ppPin = m_pConnected.Get();
        m_pConnected->AddRef();
        return S_OK;
    }

    STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE* pmt)
    {
        if (!pmt) return E_POINTER;
        if (!m_bConnected || !m_pConnected) return VFW_E_NOT_CONNECTED;
        return FillAMMediaType(&m_mt, pmt);
    }

    STDMETHODIMP QueryPinInfo(PIN_INFO* pInfo)
    {
        if (!pInfo) return E_POINTER;
        pInfo->pFilter = NULL;
        pInfo->dir = PINDIR_OUTPUT;
        wcscpy_s(pInfo->achName, MAX_PIN_NAME, m_achName);
        return S_OK;
    }

    STDMETHODIMP QueryDirection(PIN_DIRECTION* pPinDir)
    {
        if (!pPinDir) return E_POINTER;
        *pPinDir = PINDIR_OUTPUT;
        return S_OK;
    }

    STDMETHODIMP QueryId(LPWSTR* Id)
    {
        if (!Id) return E_POINTER;
        *Id = (LPWSTR)CoTaskMemAlloc(sizeof(m_achName));
        if (!*Id) return E_OUTOFMEMORY;
        wcscpy_s(*Id, MAX_PIN_NAME, m_achName);
        return S_OK;
    }

    STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE* /*pmt*/)
    {
        // Accept whatever the downstream pin proposes; the connection type was
        // already fixed by the bridge configuration.
        return S_OK;
    }

    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes** /*ppEnum*/)
    {
        // No preferred media types; the connection is driven by ReceiveConnection.
        return E_NOTIMPL;
    }

    STDMETHODIMP QueryInternalConnections(IPin** /*apPin*/, ULONG* /*nPin*/)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP EndOfStream() { return S_OK; }
    STDMETHODIMP BeginFlush() { return S_OK; }
    STDMETHODIMP EndFlush() { return S_OK; }

    STDMETHODIMP NewSegment(REFERENCE_TIME /*tStart*/, REFERENCE_TIME /*tStop*/, double /*dRate*/)
    {
        return S_OK;
    }

private:
    LONG            m_cRef;
    ComPtr<IPin>    m_pConnected;
    BOOL            m_bConnected;
    BOOL            m_bSampleTypeSent;
    AM_MEDIA_TYPE   m_mt;
    WCHAR           m_achName[MAX_PIN_NAME];
};

// ----------------------------------------------------------------------------
// CDShowPullSink -- DirectShow input pin that pulls decoded samples out of a
// DShow source filter and hands them to an MF stream (DS->MF half of bridge).
// ----------------------------------------------------------------------------
class CDShowPullSink : public IPin, public IMemInputPin
{
public:
    CDShowPullSink()
        : m_cRef(1)
        , m_pStream(NULL)
        , m_bConnected(FALSE)
    {
        ZeroMemory(&m_mt, sizeof(m_mt));
        wcscpy_s(m_achName, L"MFBridgeIn");
    }

    virtual ~CDShowPullSink()
    {
        if (m_mt.pbFormat != NULL)
            FreeAMMediaType(&m_mt);
    }

    // Ownership: the owning CDShowMediaStream sets this pointer before the
    // graph runs; the sink keeps a raw (non-refcounted) pointer to avoid a
    // reference cycle. The sink is always destroyed by the stream.
    void SetStream(CDShowMediaStream* pStream) { m_pStream = pStream; }

    const AM_MEDIA_TYPE* GetConnectedType() const { return &m_mt; }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown)
        {
            *ppv = static_cast<IPin*>(this);
            AddRef();
            return S_OK;
        }
        if (riid == IID_IPin)
        {
            *ppv = static_cast<IPin*>(this);
            AddRef();
            return S_OK;
        }
        if (riid == IID_IMemInputPin)
        {
            *ppv = static_cast<IMemInputPin*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG ulRef = InterlockedDecrement(&m_cRef);
        if (ulRef == 0) delete this;
        return ulRef;
    }

    // IPin
    STDMETHODIMP Connect(IPin* /*pReceivePin*/, const AM_MEDIA_TYPE* /*pmt*/)
    {
        // Input pins are connected by the upstream pin's Connect()/the graph
        // manager; direct Connect on an input pin is not supported.
        return E_UNEXPECTED;
    }

    STDMETHODIMP ReceiveConnection(IPin* pConnector, const AM_MEDIA_TYPE* pmt)
    {
        if (!pConnector || !pmt) return E_POINTER;
        if (m_pConnected) return VFW_E_ALREADY_CONNECTED;

        AM_MEDIA_TYPE* pCopy = NULL;
        HRESULT hr = CopyAMMediaType(pmt, &pCopy);
        if (FAILED(hr)) return hr;

        if (m_mt.pbFormat != NULL)
            FreeAMMediaType(&m_mt);
        m_mt = *pCopy;
        CoTaskMemFree(pCopy);

        m_pConnected = pConnector;
        m_bConnected = TRUE;
        return S_OK;
    }

    STDMETHODIMP Disconnect()
    {
        if (!m_bConnected || !m_pConnected) return VFW_E_NOT_CONNECTED;
        m_pConnected.Release();
        m_bConnected = FALSE;
        return S_OK;
    }

    STDMETHODIMP ConnectedTo(IPin** ppPin)
    {
        if (!ppPin) return E_POINTER;
        if (!m_bConnected || !m_pConnected) return VFW_E_NOT_CONNECTED;
        *ppPin = m_pConnected.Get();
        m_pConnected->AddRef();
        return S_OK;
    }

    STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE* pmt)
    {
        if (!pmt) return E_POINTER;
        if (!m_bConnected || !m_pConnected) return VFW_E_NOT_CONNECTED;
        return FillAMMediaType(&m_mt, pmt);
    }

    STDMETHODIMP QueryPinInfo(PIN_INFO* pInfo)
    {
        if (!pInfo) return E_POINTER;
        pInfo->pFilter = NULL;
        pInfo->dir = PINDIR_INPUT;
        wcscpy_s(pInfo->achName, MAX_PIN_NAME, m_achName);
        return S_OK;
    }

    STDMETHODIMP QueryDirection(PIN_DIRECTION* pPinDir)
    {
        if (!pPinDir) return E_POINTER;
        *pPinDir = PINDIR_INPUT;
        return S_OK;
    }

    STDMETHODIMP QueryId(LPWSTR* Id)
    {
        if (!Id) return E_POINTER;
        *Id = (LPWSTR)CoTaskMemAlloc(sizeof(m_achName));
        if (!*Id) return E_OUTOFMEMORY;
        wcscpy_s(*Id, MAX_PIN_NAME, m_achName);
        return S_OK;
    }

    STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE* /*pmt*/)
    {
        // The bridge is format-agnostic: accept whatever the source offers.
        return S_OK;
    }

    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes** /*ppEnum*/)
    {
        // No preferred media types; we accept the negotiated source type.
        return E_NOTIMPL;
    }

    STDMETHODIMP QueryInternalConnections(IPin** /*apPin*/, ULONG* /*nPin*/)
    {
        return E_NOTIMPL;
    }

    // These are defined out-of-line because they call back into the owning
    // CDShowMediaStream (defined after this class).
    STDMETHODIMP EndOfStream();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    // IMemInputPin
    STDMETHODIMP GetAllocator(IMemAllocator** ppAllocator)
    {
        // We do not provide an allocator; the upstream filter supplies one.
        if (ppAllocator) *ppAllocator = NULL;
        return E_NOTIMPL;
    }

    STDMETHODIMP NotifyAllocator(IMemAllocator* /*pAllocator*/, BOOL /*bReadOnly*/)
    {
        return S_OK;
    }

    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProps)
    {
        if (!pProps) return E_POINTER;
        ZeroMemory(pProps, sizeof(ALLOCATOR_PROPERTIES));
        return S_OK;
    }

    STDMETHODIMP Receive(IMediaSample* pSample);

    STDMETHODIMP ReceiveMultiple(IMediaSample** pSamples, long nSamples, long* nSamplesProcessed)
    {
        if (!pSamples || !nSamplesProcessed) return E_POINTER;
        long n = 0;
        HRESULT hr = S_OK;
        for (long i = 0; i < nSamples; i++)
        {
            hr = Receive(pSamples[i]);
            if (FAILED(hr)) break;
            n++;
        }
        *nSamplesProcessed = n;
        return hr;
    }

    STDMETHODIMP ReceiveCanBlock()
    {
        // Receive never blocks; the bridge drops samples when its buffer is full.
        return S_FALSE;
    }

private:
    LONG                    m_cRef;
    CDShowMediaStream*      m_pStream;
    ComPtr<IPin>            m_pConnected;
    BOOL                    m_bConnected;
    AM_MEDIA_TYPE           m_mt;
    WCHAR                   m_achName[MAX_PIN_NAME];
};

// ----------------------------------------------------------------------------
// CDShowMediaStream -- MF stream that forwards decoded DShow samples to the MF
// pipeline. Each stream owns the CDShowPullSink it is fed from.
// ----------------------------------------------------------------------------
class CDShowMediaStream : public IMFMediaStream
{
public:
    enum State
    {
        StateStopped = 0,
        StateStarted,
        StatePaused
    };

    CDShowMediaStream(CDShowMediaSource* pSource, IMFStreamDescriptor* pDescriptor,
                      CDShowPullSink* pSink)
        : m_cRef(1)
        , m_pSource(pSource)
        , m_state(StateStopped)
        , m_bEndOfStream(FALSE)
        , m_bEosEventSent(FALSE)
        , m_bShutdown(FALSE)
    {
        InitializeCriticalSection(&m_lock);
        m_spDescriptor = pDescriptor;
        m_spSink = pSink;
        if (m_spSink)
            m_spSink->SetStream(this);
        MFCreateEventQueue(&m_spQueue);
    }

    virtual ~CDShowMediaStream()
    {
        DeleteCriticalSection(&m_lock);
    }

    void SetState(State state)
    {
        AutoLock lock(&m_lock);
        m_state = state;
        if (m_spQueue)
        {
            if (state == StateStarted)
                m_spQueue->QueueEventParamVar(MEStreamStarted, GUID_NULL, S_OK, NULL);
            else if (state == StatePaused)
                m_spQueue->QueueEventParamVar(MEStreamPaused, GUID_NULL, S_OK, NULL);
            else
                m_spQueue->QueueEventParamVar(MEStreamStopped, GUID_NULL, S_OK, NULL);
        }
    }

    void Shutdown()
    {
        AutoLock lock(&m_lock);
        if (m_bShutdown) return;
        m_bShutdown = TRUE;
        m_state = StateStopped;
        m_samples.clear();
        m_tokens.clear();
        if (m_spQueue)
        {
            m_spQueue->Shutdown();
            m_spQueue.Release();
        }
        m_spDescriptor.Release();
        m_spSink.Release();
    }

    // Re-arms the stream for another playback pass (Start after Stop).
    void Reset()
    {
        AutoLock lock(&m_lock);
        m_bEndOfStream = FALSE;
        m_bEosEventSent = FALSE;
        m_samples.clear();
        m_tokens.clear();
    }

    // Called by CDShowPullSink::Receive on the DShow delivery thread.
    // (Defined out-of-line after CDShowMediaSource because it calls back into
    // the owning source.)
    HRESULT DeliverDShowSample(IMediaSample* pDSSample);

    // Called by CDShowPullSink::EndOfStream.
    void OnDShowEndOfStream();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IMFMediaEventGenerator || riid == IID_IMFMediaStream)
        {
            *ppv = static_cast<IMFMediaStream*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG ulRef = InterlockedDecrement(&m_cRef);
        if (ulRef == 0) delete this;
        return ulRef;
    }

    // IMFMediaEventGenerator
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->GetEvent(dwFlags, ppEvent);
    }

    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->BeginGetEvent(pCallback, punkState);
    }

    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->EndGetEvent(pResult, ppEvent);
    }

    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus,
                            const PROPVARIANT* pvValue)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    // IMFMediaStream
    STDMETHODIMP GetMediaSource(IMFMediaSource** ppMediaSource);

    STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor)
    {
        if (!ppStreamDescriptor) return E_POINTER;
        *ppStreamDescriptor = NULL;
        if (m_bShutdown) return MF_E_SHUTDOWN;
        if (!m_spDescriptor) return E_UNEXPECTED;
        *ppStreamDescriptor = m_spDescriptor.Get();
        m_spDescriptor->AddRef();
        return S_OK;
    }

    STDMETHODIMP RequestSample(IUnknown* pToken);

private:
    enum { kMaxBufferedSamples = 64 };

    // Delivers buffered samples to pending requests. Must be called with the
    // stream lock held. Returns TRUE when the end-of-stream event was queued
    // (the caller then notifies the owning source outside the lock).
    BOOL DeliverPendingLocked()
    {
        while (!m_samples.empty() && !m_tokens.empty())
        {
            ComPtr<IMFSample> spSample = m_samples.front();
            m_samples.pop_front();

            ComPtr<IUnknown> spToken = m_tokens.front();
            m_tokens.pop_front();

            if (spToken)
                spSample->SetUnknown(MFSampleExtension_Token, spToken);

            HRESULT hr = m_spQueue ? m_spQueue->QueueEventParamUnk(
                MEMediaSample, GUID_NULL, S_OK, spSample) : E_FAIL;
            if (FAILED(hr))
                return FALSE;
        }

        if (m_bEndOfStream && m_samples.empty() && !m_bEosEventSent)
        {
            m_bEosEventSent = TRUE;
            while (!m_tokens.empty())
                m_tokens.pop_front(); // unfulfillable requests are dropped
            if (m_spQueue)
                m_spQueue->QueueEventParamVar(MEEndOfStream, GUID_NULL, S_OK, NULL);
            return TRUE;
        }
        return FALSE;
    }

    LONG                    m_cRef;
    CDShowMediaSource*      m_pSource;
    CRITICAL_SECTION        m_lock;
    ComPtr<IMFMediaEventQueue>  m_spQueue;
    ComPtr<IMFStreamDescriptor> m_spDescriptor;
    ComPtr<CDShowPullSink>  m_spSink;
    std::deque<ComPtr<IMFSample>>   m_samples;
    std::deque<ComPtr<IUnknown>>    m_tokens;
    State                   m_state;
    BOOL                    m_bEndOfStream;
    BOOL                    m_bEosEventSent;
    BOOL                    m_bShutdown;
};

// ----------------------------------------------------------------------------
// CDShowMediaSource -- IMFMediaSource that owns the DShow filter graph, one
// CDShowMediaStream per connected source output pin, and the presentation
// descriptor describing those streams.
// ----------------------------------------------------------------------------
class CDShowMediaSource : public IMFMediaSource
{
public:
    enum State
    {
        StateStopped = 0,
        StateStarted,
        StatePaused,
        StateShutdown
    };

    CDShowMediaSource(IGraphBuilder* pGraph, IBaseFilter* pSourceFilter,
                      IMFPresentationDescriptor* pPresentation)
        : m_cRef(1)
        , m_state(StateStopped)
        , m_dwStreamsEnded(0)
    {
        InitializeCriticalSection(&m_lock);
        m_spGraph = pGraph;
        m_spSourceFilter = pSourceFilter;
        m_spPresentation = pPresentation;
        MFCreateEventQueue(&m_spQueue);

        // The graph manager implements IMediaFilter; use it to Run/Stop/Pause
        // the filter graph (IGraphBuilder itself has no Run/Stop/Pause).
        if (m_spGraph)
            m_spGraph->QueryInterface(IID_IMediaFilter,
                reinterpret_cast<void**>(&m_spGraphControl));
    }

    virtual ~CDShowMediaSource()
    {
        DeleteCriticalSection(&m_lock);
    }

    void AddStream(CDShowMediaStream* pStream)
    {
        if (!pStream) return;
        AutoLock lock(&m_lock);
        m_streams.push_back(ComPtr<CDShowMediaStream>(pStream));
    }

    void OnStreamEndOfStream(CDShowMediaStream* pStream)
    {
        UNREFERENCED_PARAMETER(pStream);
        AutoLock lock(&m_lock);
        if (m_state == StateShutdown) return;
        m_dwStreamsEnded++;
        if (m_dwStreamsEnded >= m_streams.size() && m_spQueue)
        {
            // All streams finished: raise the end-of-presentation event.
            m_spQueue->QueueEventParamVar(MEEndOfPresentation, GUID_NULL, S_OK, NULL);
        }
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IMFMediaEventGenerator || riid == IID_IMFMediaSource)
        {
            *ppv = static_cast<IMFMediaSource*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG ulRef = InterlockedDecrement(&m_cRef);
        if (ulRef == 0) delete this;
        return ulRef;
    }

    // IMFMediaEventGenerator
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->GetEvent(dwFlags, ppEvent);
    }

    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->BeginGetEvent(pCallback, punkState);
    }

    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->EndGetEvent(pResult, ppEvent);
    }

    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus,
                            const PROPVARIANT* pvValue)
    {
        if (!m_spQueue) return MF_E_SHUTDOWN;
        return m_spQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    // IMFMediaSource
    STDMETHODIMP GetCharacteristics(DWORD* pdwCharacteristics)
    {
        if (!pdwCharacteristics) return E_POINTER;
        if (m_state == StateShutdown) return MF_E_SHUTDOWN;
        // Pausing maps to the DShow graph's Pause; seeking is out of scope.
        *pdwCharacteristics = MFMEDIASOURCE_CAN_PAUSE;
        return S_OK;
    }

    STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor)
    {
        if (!ppPresentationDescriptor) return E_POINTER;
        *ppPresentationDescriptor = NULL;
        if (m_state == StateShutdown) return MF_E_SHUTDOWN;
        if (!m_spPresentation) return E_UNEXPECTED;
        *ppPresentationDescriptor = m_spPresentation.Get();
        m_spPresentation->AddRef();
        return S_OK;
    }

    STDMETHODIMP Start(IMFPresentationDescriptor* /*pPresentationDescriptor*/,
                       const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition)
    {
        if (m_state == StateShutdown) return MF_E_SHUTDOWN;
        if (pguidTimeFormat && *pguidTimeFormat != GUID_NULL)
            return MF_E_UNSUPPORTED_TIME_FORMAT;

        // Seeking is not implemented: a non-empty start position restarts the
        // DShow source from its beginning. (Out of scope: DShow->MF seeking.)
        UNREFERENCED_PARAMETER(pvarStartPosition);

        HRESULT hr = S_OK;
        if (m_spGraphControl)
        {
            if (m_state == StateStarted)
                return S_OK; // already running
            hr = m_spGraphControl->Run(0);
            if (FAILED(hr)) return hr;
        }

        m_state = StateStarted;
        m_dwStreamsEnded = 0;

        if (m_spQueue)
            m_spQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, S_OK, pvarStartPosition);

        // Notify the streams outside the source lock (lock ordering: never
        // hold the source lock while acquiring a stream lock).
        for (size_t i = 0; i < m_streams.size(); i++)
        {
            m_streams[i]->Reset();
            m_streams[i]->SetState(CDShowMediaStream::StateStarted);
        }

        return S_OK;
    }

    STDMETHODIMP Stop()
    {
        if (m_state == StateShutdown) return MF_E_SHUTDOWN;

        if (m_spGraphControl)
            m_spGraphControl->Stop();
        m_state = StateStopped;

        if (m_spQueue)
            m_spQueue->QueueEventParamVar(MESourceStopped, GUID_NULL, S_OK, NULL);

        for (size_t i = 0; i < m_streams.size(); i++)
            m_streams[i]->SetState(CDShowMediaStream::StateStopped);

        return S_OK;
    }

    STDMETHODIMP Pause()
    {
        if (m_state == StateShutdown) return MF_E_SHUTDOWN;

        if (m_spGraphControl)
            m_spGraphControl->Pause();
        m_state = StatePaused;

        if (m_spQueue)
            m_spQueue->QueueEventParamVar(MESourcePaused, GUID_NULL, S_OK, NULL);

        for (size_t i = 0; i < m_streams.size(); i++)
            m_streams[i]->SetState(CDShowMediaStream::StatePaused);

        return S_OK;
    }

    STDMETHODIMP Shutdown()
    {
        AutoLock lock(&m_lock);
        if (m_state == StateShutdown) return MF_E_SHUTDOWN;
        m_state = StateShutdown;

        if (m_spGraphControl)
            m_spGraphControl->Stop();

        for (size_t i = 0; i < m_streams.size(); i++)
            m_streams[i]->Shutdown();
        m_streams.clear();

        if (m_spQueue)
        {
            m_spQueue->Shutdown();
            m_spQueue.Release();
        }
        m_spPresentation.Release();
        m_spGraphControl.Release();
        m_spGraph.Release();
        m_spSourceFilter.Release();
        return S_OK;
    }

private:
    LONG                                m_cRef;
    CRITICAL_SECTION                    m_lock;
    ComPtr<IMFMediaEventQueue>          m_spQueue;
    ComPtr<IMFPresentationDescriptor>   m_spPresentation;
    ComPtr<IGraphBuilder>               m_spGraph;
    ComPtr<IMediaFilter>                m_spGraphControl;
    ComPtr<IBaseFilter>                 m_spSourceFilter;
    std::vector<ComPtr<CDShowMediaStream> > m_streams;
    State                               m_state;
    DWORD                               m_dwStreamsEnded;
};

// ----------------------------------------------------------------------------
// CDShowMediaStream out-of-line methods. These call back into the owning
// CDShowMediaSource and must be defined after that class is complete. None of
// them holds the stream lock while calling into the source (lock ordering).
// ----------------------------------------------------------------------------
HRESULT CDShowMediaStream::DeliverDShowSample(IMediaSample* pDSSample)
{
    if (!pDSSample) return E_POINTER;

    ComPtr<IMFSample> spSample;
    HRESULT hr = ConvertDShowSampleToMF(pDSSample, &spSample);
    if (FAILED(hr)) return hr;

    BOOL bNotifySource = FALSE;
    {
        AutoLock lock(&m_lock);
        if (m_bShutdown || m_bEndOfStream) return S_OK;

        if (m_tokens.empty())
        {
            // No pending requests; buffer up to a limit, dropping the oldest
            // sample once the buffer is full.
            if (m_samples.size() >= kMaxBufferedSamples)
                m_samples.pop_front();
            m_samples.push_back(spSample);
            return S_OK;
        }
        bNotifySource = DeliverPendingLocked();
    }

    if (bNotifySource && m_pSource)
        m_pSource->OnStreamEndOfStream(this);
    return S_OK;
}

void CDShowMediaStream::OnDShowEndOfStream()
{
    BOOL bNotifySource = FALSE;
    {
        AutoLock lock(&m_lock);
        m_bEndOfStream = TRUE;
        bNotifySource = DeliverPendingLocked();
    }
    if (bNotifySource && m_pSource)
        m_pSource->OnStreamEndOfStream(this);
}

STDMETHODIMP CDShowMediaStream::GetMediaSource(IMFMediaSource** ppMediaSource)
{
    if (!ppMediaSource) return E_POINTER;
    *ppMediaSource = NULL;
    if (!m_pSource) return E_UNEXPECTED;
    *ppMediaSource = m_pSource;
    m_pSource->AddRef();
    return S_OK;
}

STDMETHODIMP CDShowMediaStream::RequestSample(IUnknown* pToken)
{
    BOOL bNotifySource = FALSE;
    {
        AutoLock lock(&m_lock);
        if (m_bShutdown) return MF_E_SHUTDOWN;
        if (m_state == StateStopped) return MF_E_MEDIA_SOURCE_WRONGSTATE;
        if (m_bEndOfStream && m_samples.empty()) return MF_E_END_OF_STREAM;

        m_tokens.push_back(ComPtr<IUnknown>(pToken));
        bNotifySource = DeliverPendingLocked();
    }
    if (bNotifySource && m_pSource)
        m_pSource->OnStreamEndOfStream(this);
    return S_OK;
}

// ----------------------------------------------------------------------------
// CDShowPullSink out-of-line methods that call back into the owning stream.
// ----------------------------------------------------------------------------
STDMETHODIMP CDShowPullSink::EndOfStream()
{
    if (m_pStream)
        m_pStream->OnDShowEndOfStream();
    return S_OK;
}

STDMETHODIMP CDShowPullSink::BeginFlush()
{
    return S_OK;
}

STDMETHODIMP CDShowPullSink::EndFlush()
{
    return S_OK;
}

STDMETHODIMP CDShowPullSink::NewSegment(REFERENCE_TIME /*tStart*/, REFERENCE_TIME /*tStop*/, double /*dRate*/)
{
    return S_OK;
}

STDMETHODIMP CDShowPullSink::Receive(IMediaSample* pSample)
{
    if (!m_pStream) return E_UNEXPECTED;
    return m_pStream->DeliverDShowSample(pSample);
}

// ----------------------------------------------------------------------------
// DShow source creation -- resolves pszDShowFilter as one of:
//   1. a registered filter CLSID string ("{...}"),
//   2. a media file path (absolute or relative),
//   3. a registered DirectShow filter friendly name.
// ----------------------------------------------------------------------------
HRESULT CreateDShowSource(LPCWSTR pszFilter, IGraphBuilder* pGraph, IBaseFilter** ppSource)
{
    if (!pszFilter || !pGraph || !ppSource) return E_POINTER;
    *ppSource = NULL;

    // 1) Filter CLSID string.
    GUID clsid;
    if (CLSIDFromString(pszFilter, &clsid) == S_OK)
    {
        ComPtr<IBaseFilter> spFilter;
        HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&spFilter));
        if (FAILED(hr)) return hr;
        hr = pGraph->AddFilter(spFilter, L"WLMFDS Bridge Source");
        if (FAILED(hr)) return hr;
        *ppSource = spFilter.Detach();
        return S_OK;
    }

    // 2) Media file path.
    BOOL bLooksLikePath = (PathIsRelativeW(pszFilter) == TRUE) ||
        (GetFileAttributesW(pszFilter) != INVALID_FILE_ATTRIBUTES) ||
        (wcschr(pszFilter, L'\\') != NULL) || (wcschr(pszFilter, L'/') != NULL);
    if (bLooksLikePath)
    {
        ComPtr<IBaseFilter> spFilter;
        HRESULT hr = pGraph->AddSourceFilter(pszFilter, L"WLMFDS Bridge Source", &spFilter);
        if (SUCCEEDED(hr))
        {
            *ppSource = spFilter.Detach();
            return S_OK;
        }
        // Not a resolvable path; fall through to friendly-name lookup.
    }

    // 3) Registered filter friendly name (IFilterMapper enumeration).
    ComPtr<IFilterMapper> spMapper;
    HRESULT hr = CoCreateInstance(CLSID_FilterMapper, NULL, CLSCTX_INPROC_SERVER,
        IID_IFilterMapper, reinterpret_cast<void**>(&spMapper));
    if (FAILED(hr)) return hr;

    GUID guidNull = GUID_NULL;
    ComPtr<IEnumRegFilters> spEnum;
    hr = spMapper->EnumMatchingFilters(&spEnum, 0, FALSE, guidNull, guidNull,
        FALSE, FALSE, guidNull, guidNull);
    if (FAILED(hr)) return hr;

    REGFILTER* pReg = NULL;
    while (spEnum->Next(1, &pReg, NULL) == S_OK)
    {
        BOOL bMatch = (pReg->Name != NULL) && (lstrcmpiW(pReg->Name, pszFilter) == 0);
        CLSID clsidFilter = pReg->Clsid;
        if (pReg->Name) CoTaskMemFree(pReg->Name);
        CoTaskMemFree(pReg);
        pReg = NULL;

        if (!bMatch) continue;

        ComPtr<IBaseFilter> spFilter;
        hr = CoCreateInstance(clsidFilter, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&spFilter));
        if (FAILED(hr)) return hr;
        hr = pGraph->AddFilter(spFilter, L"WLMFDS Bridge Source");
        if (FAILED(hr)) return hr;
        *ppSource = spFilter.Detach();
        return S_OK;
    }

    return VFW_E_CANNOT_CONNECT;
}

// ============================================================================
// EVRPresenter -- Enhanced Video Renderer integration
// ============================================================================
class EVRPresenter
{
public:
    EVRPresenter()
        : m_pEVR(NULL)
        , m_pDisplayControl(NULL)
    {
    }

    ~EVRPresenter()
    {
        Release();
    }

    HRESULT Initialize(HWND hWnd, UINT32 uWidth, UINT32 uHeight)
    {
        // Create the EVR DirectShow filter.
        HRESULT hr = CoCreateInstance(
            CLSID_EnhancedVideoRenderer, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pEVR));
        if (FAILED(hr))
            return hr;

        // The DShow EVR filter exposes IMFVideoDisplayControl (not
        // IMFVideoPresenter -- that is implemented by the presenter object
        // the EVR instantiates internally).
        hr = m_pEVR->QueryInterface(IID_IMFVideoDisplayControl,
            reinterpret_cast<void**>(&m_pDisplayControl));
        if (FAILED(hr))
        {
            Release();
            return hr;
        }

        if (hWnd != NULL)
            m_pDisplayControl->SetVideoWindow(hWnd);

        // Pre-size the mixer's output to the configured video size.
        MFVideoNormalizedRect nrc = { 0.0f, 0.0f, 1.0f, 1.0f };
        RECT rcDest = { 0, 0, (LONG)uWidth, (LONG)uHeight };
        m_pDisplayControl->SetVideoPosition(&nrc, &rcDest);
        m_pDisplayControl->SetAspectRatioMode(MFVideoARMode_PreservePicture);

        return S_OK;
    }

    void Release()
    {
        if (m_pDisplayControl) { m_pDisplayControl->Release(); m_pDisplayControl = NULL; }
        if (m_pEVR)            { m_pEVR->Release();            m_pEVR = NULL; }
    }

private:
    IBaseFilter*            m_pEVR;
    IMFVideoDisplayControl* m_pDisplayControl;
};

// ============================================================================
// DShowToMFConverter -- converts DShow sources to MF-compatible streams
// ============================================================================
class DShowToMFConverter
{
public:
    DShowToMFConverter()
        : m_hReadyEvent(NULL)
    {
    }

    ~DShowToMFConverter()
    {
    }

    HRESULT Convert(LPCWSTR pszDShowFilter, HANDLE hMFReader)
    {
        m_hReadyEvent = hMFReader;
        m_spMediaSource.Release();

        if (!pszDShowFilter || !pszDShowFilter[0])
            return E_INVALIDARG;

        ComPtr<IMFMediaSource> spMediaSource;

        // Preferred path: let the MF Source Resolver handle anything MF can
        // parse natively (this mirrors the resolver import of the original
        // DLL). Only when that fails do we fall back to the DShow bridge.
        if (GetFileAttributesW(pszDShowFilter) != INVALID_FILE_ATTRIBUTES ||
            wcschr(pszDShowFilter, L'.') != NULL ||
            PathIsRelativeW(pszDShowFilter) == TRUE)
        {
            ComPtr<IMFSourceResolver> spResolver;
            if (SUCCEEDED(MFCreateSourceResolver(&spResolver)))
            {
                MF_OBJECT_TYPE objType = MF_OBJECT_INVALID;
                IUnknown* pUnk = NULL;
                HRESULT hrResolve = spResolver->CreateObjectFromURL(
                    pszDShowFilter, MF_RESOLUTION_MEDIASOURCE,
                    NULL, &objType, &pUnk);
                if (SUCCEEDED(hrResolve) && pUnk != NULL)
                {
                    pUnk->QueryInterface(IID_IMFMediaSource,
                        reinterpret_cast<void**>(&spMediaSource));
                    pUnk->Release();
                }
            }
        }

        if (!spMediaSource)
        {
            // 1. Create a DirectShow filter graph with the specified filter.
            ComPtr<IGraphBuilder> spGraph;
            HRESULT hr = CoCreateInstance(
                CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                IID_IGraphBuilder, reinterpret_cast<void**>(&spGraph));
            if (FAILED(hr))
                return hr;

            ComPtr<IBaseFilter> spSource;
            hr = CreateDShowSource(pszDShowFilter, spGraph, &spSource);
            if (FAILED(hr))
                return hr;

            // 2. Capture the output pins: connect each to a pull sink and
            //    negotiate the media types.
            ComPtr<IEnumPins> spEnumPins;
            hr = spSource->EnumPins(&spEnumPins);
            if (FAILED(hr))
                return hr;

            std::vector<ComPtr<CDShowPullSink> > sinks;
            std::vector<ComPtr<IMFStreamDescriptor> > descriptors;

            IPin* pPin = NULL;
            DWORD dwIndex = 0;
            while (spEnumPins->Next(1, &pPin, NULL) == S_OK)
            {
                PIN_DIRECTION dir;
                pPin->QueryDirection(&dir);

                if (dir == PINDIR_OUTPUT)
                {
                    ComPtr<CDShowPullSink> spSink(new CDShowPullSink());
                    HRESULT hrConnect = pPin->Connect(spSink, NULL);
                    if (FAILED(hrConnect) && spGraph)
                    {
                        // Fall back to the graph manager's connection logic.
                        hrConnect = spGraph->ConnectDirect(pPin, spSink, NULL);
                    }

                    if (SUCCEEDED(hrConnect))
                    {
                        // Convert the negotiated DShow type to an MF type.
                        ComPtr<IMFMediaType> spMFType;
                        if (SUCCEEDED(CreateMFTypeFromAMType(
                                spSink->GetConnectedType(), &spMFType)))
                        {
                            IMFMediaType* apTypes[1] = { spMFType.Get() };
                            ComPtr<IMFStreamDescriptor> spDescriptor;
                            hr = MFCreateStreamDescriptor(dwIndex, 1, apTypes, &spDescriptor);
                            if (SUCCEEDED(hr))
                            {
                                sinks.push_back(spSink);
                                descriptors.push_back(spDescriptor);
                            }
                        }
                    }
                }

                pPin->Release();
                pPin = NULL;
                dwIndex++;
            }

            if (descriptors.empty())
                return VFW_E_CANNOT_CONNECT;

            // 3. Wrap the graph + pins in an MF media source and return the
            //    MF source handle.
            std::vector<IMFStreamDescriptor*> apRawDescriptors;
            for (size_t i = 0; i < descriptors.size(); i++)
                apRawDescriptors.push_back(descriptors[i].Get());

            ComPtr<IMFPresentationDescriptor> spPresentation;
            hr = MFCreatePresentationDescriptor(
                (DWORD)apRawDescriptors.size(),
                apRawDescriptors.empty() ? NULL : &apRawDescriptors[0],
                &spPresentation);
            if (FAILED(hr))
                return hr;

            ComPtr<CDShowMediaSource> spBridgeSource(
                new CDShowMediaSource(spGraph, spSource, spPresentation));

            for (size_t i = 0; i < sinks.size(); i++)
            {
                ComPtr<CDShowMediaStream> spStream(
                    new CDShowMediaStream(spBridgeSource, descriptors[i], sinks[i]));
                spBridgeSource->AddStream(spStream);
            }

            spMediaSource = spBridgeSource.Get(); // upcast to IMFMediaSource
        }

        m_spMediaSource = spMediaSource;

        // 4. Signal the caller (if an event handle was provided) that the MF
        //    source is fully constructed and ready to be started.
        if (m_hReadyEvent != NULL)
            SetEvent(m_hReadyEvent);

        return S_OK;
    }

    HRESULT GetMediaSource(IMFMediaSource** ppSource)
    {
        if (!ppSource) return E_POINTER;
        *ppSource = NULL;
        if (!m_spMediaSource) return E_UNEXPECTED;
        *ppSource = m_spMediaSource.Get();
        m_spMediaSource->AddRef();
        return S_OK;
    }

private:
    HANDLE                  m_hReadyEvent;
    ComPtr<IMFMediaSource>  m_spMediaSource;
};

// ============================================================================
// BridgeManager -- coordinates the DShow-to-MF bridge
// ============================================================================
class BridgeManager
{
public:
    BridgeManager()
        : m_bInitialized(false)
    {
    }

    ~BridgeManager()
    {
        Shutdown();
    }

    HRESULT Initialize(const MFBridgeConfig* pConfig)
    {
        if (!pConfig)
            return E_INVALIDARG;

        m_config = *pConfig;

        if (pConfig->bUseEVR)
        {
            m_evrPresenter.reset(new EVRPresenter());
            HRESULT hr = m_evrPresenter->Initialize(
                pConfig->hVideoWindow, pConfig->uVideoWidth, pConfig->uVideoHeight);
            // EVR init is optional
            UNREFERENCED_PARAMETER(hr);
        }

        m_bInitialized = true;
        return S_OK;
    }

    void Shutdown()
    {
        m_evrPresenter.reset();
        m_converter.reset();
        m_spEVRVideoOutput.Release();
        m_spMediaSource.Release();
        m_bInitialized = false;
    }

    HRESULT ConvertDShowSource(LPCWSTR pszDShowFilter, HANDLE hMFReader)
    {
        if (!m_converter)
            m_converter.reset(new DShowToMFConverter());

        HRESULT hr = m_converter->Convert(pszDShowFilter, hMFReader);
        if (FAILED(hr))
            return hr;

        m_spMediaSource.Release();
        hr = m_converter->GetMediaSource(&m_spMediaSource);
        return hr;
    }

    HRESULT GetMediaSource(IMFMediaSource** ppSource)
    {
        if (!ppSource) return E_POINTER;
        *ppSource = NULL;
        if (!m_spMediaSource) return E_UNEXPECTED;
        *ppSource = m_spMediaSource.Get();
        m_spMediaSource->AddRef();
        return S_OK;
    }

    // MF -> DS: deliver a decoded MF video sample into the connected EVR.
    HRESULT DeliverVideoFrame(IMFSample* pSample)
    {
        if (!pSample) return E_INVALIDARG;
        if (!m_spEVRVideoOutput) return VFW_E_NOT_CONNECTED;
        return m_spEVRVideoOutput->DeliverSample(pSample);
    }

    HRESULT SetupEVR(IBaseFilter* pEVRFilter)
    {
        if (!pEVRFilter)
            return E_INVALIDARG;

        // Locate the EVR input pin.
        ComPtr<IEnumPins> spEnumPins;
        HRESULT hr = pEVRFilter->EnumPins(&spEnumPins);
        if (FAILED(hr))
            return hr;

        ComPtr<IPin> spEVRInputPin;
        IPin* pPin = NULL;
        while (spEnumPins->Next(1, &pPin, NULL) == S_OK)
        {
            PIN_DIRECTION dir;
            pPin->QueryDirection(&dir);
            if (dir == PINDIR_INPUT)
            {
                spEVRInputPin = pPin;
                pPin->Release();
                pPin = NULL;
                break;
            }
            pPin->Release();
            pPin = NULL;
        }
        if (!spEVRInputPin)
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        // Build the bridge output pin and connect it to the EVR input pin.
        AMMediaTypeGuard mtGuard;
        AM_MEDIA_TYPE* pmt = NULL;
        hr = BuildAMVideoType(m_config.uVideoWidth, m_config.uVideoHeight,
                              MEDIASUBTYPE_NV12, &pmt);
        if (FAILED(hr))
            return hr;
        mtGuard.Reset(pmt);

        m_spEVRVideoOutput.Release();
        m_spEVRVideoOutput.Attach(new CMFVideoOutputPin());
        if (!m_spEVRVideoOutput)
            return E_OUTOFMEMORY;

        hr = m_spEVRVideoOutput->ConnectToInput(spEVRInputPin, mtGuard.p);
        if (FAILED(hr))
            return hr;

        // Run the EVR so it can present frames as they arrive.
        ComPtr<IMediaFilter> spMediaFilter;
        if (SUCCEEDED(pEVRFilter->QueryInterface(
                IID_IMediaFilter, reinterpret_cast<void**>(&spMediaFilter))))
        {
            spMediaFilter->SetSyncSource(NULL); // use the EVR's own clock
            spMediaFilter->Run(0);
        }

        // Point the EVR at the destination window.
        ComPtr<IMFVideoDisplayControl> spDisplayControl;
        if (SUCCEEDED(pEVRFilter->QueryInterface(
                IID_IMFVideoDisplayControl,
                reinterpret_cast<void**>(&spDisplayControl))))
        {
            if (m_config.hVideoWindow != NULL)
                spDisplayControl->SetVideoWindow(m_config.hVideoWindow);
            spDisplayControl->SetAspectRatioMode(MFVideoARMode_PreservePicture);
        }

        return S_OK;
    }

private:
    bool                                        m_bInitialized;
    MFBridgeConfig                              m_config;
    std::unique_ptr<EVRPresenter>               m_evrPresenter;
    std::unique_ptr<DShowToMFConverter>         m_converter;
    ComPtr<IMFMediaSource>                      m_spMediaSource;
    ComPtr<CMFVideoOutputPin>                   m_spEVRVideoOutput;
};

} // namespace MFDS

// ============================================================================
// Exported functions -- standard COM DLL entry points
// ============================================================================

extern "C"
{

STDAPI DllCanUnloadNow()
{
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(ppv);
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllRegisterServer()
{
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    return S_OK;
}

} // extern "C"
