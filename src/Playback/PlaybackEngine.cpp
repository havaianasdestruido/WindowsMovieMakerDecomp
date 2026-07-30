#include "PlaybackEngine.h"
#include "Renderer.h"
#include <mfapi.h>
#include <mferror.h>
#include <d3d11.h>

template <class T>
inline void SafeReleasePlayback(T*& pVal) {
    if (pVal != nullptr) {
        pVal->Release();
        pVal = nullptr;
    }
}

namespace DirectUI {

PlaybackEngine::PlaybackEngine(GPURenderer* pRenderer)
    : m_pRenderer(pRenderer)
{
    MFStartup(MF_VERSION);
}

PlaybackEngine::~PlaybackEngine() {
    Cleanup();
    MFShutdown();
}

void PlaybackEngine::Cleanup() {
    Stop();
    SafeReleasePlayback(m_pFrameTexture);
    SafeReleasePlayback(m_pReader);
}

HRESULT PlaybackEngine::OpenFile(const std::wstring& path) {
    Cleanup();
    m_eof = false;
    m_currentTime = 0.0;
    m_duration = 0.0;

    HRESULT hr = MFCreateSourceReaderFromURL(
        path.c_str(), nullptr, &m_pReader);
    if (FAILED(hr)) return hr;

    IMFMediaType* pVideoType = nullptr;
    for (DWORD i = 0; ; ++i) {
        IMFMediaType* pType = nullptr;
        hr = m_pReader->GetNativeMediaType(i, 0, &pType);
        if (FAILED(hr)) break;

        GUID majorType;
        pType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
        if (majorType == MFMediaType_Video) {
            pVideoType = pType;
            break;
        }
        SafeReleasePlayback(pType);
    }

    if (!pVideoType) return MF_E_INVALIDMEDIATYPE;

    m_pReader->SetCurrentMediaType(0, nullptr, pVideoType);

    PROPVARIANT var;
    hr = m_pReader->GetCurrentMediaType(0, &pVideoType);
    if (SUCCEEDED(hr)) {
        UINT32 width = 0, height = 0;
        MFGetAttributeSize(pVideoType, MF_MT_FRAME_SIZE, &width, &height);

        if (m_pRenderer && m_pRenderer->GetDevice()) {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width = width > 0 ? width : 1920;
            desc.Height = height > 0 ? height : 1080;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = 0;

            m_pRenderer->GetDevice()->CreateTexture2D(&desc, nullptr, &m_pFrameTexture);
        }
        SafeReleasePlayback(pVideoType);
    }

    PROPVARIANT varDuration;
    hr = m_pReader->GetPresentationAttribute(
        MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION,
        &varDuration);
    if (SUCCEEDED(hr)) {
        m_duration = (double)varDuration.uhVal.QuadPart / 10000000.0;
    }

    return S_OK;
}

HRESULT PlaybackEngine::Play() {
    if (!m_pReader) return MF_E_NOT_INITIALIZED;
    if (m_eof) {
        m_eof = false;
        Seek(0.0);
    }
    m_playing = true;
    m_tickStart = GetTickCount() - (DWORD)(m_tickPauseOffset);
    return S_OK;
}

HRESULT PlaybackEngine::Pause() {
    if (!m_playing) return S_OK;
    m_tickPauseOffset = GetTickCount() - m_tickStart;
    m_playing = false;
    return S_OK;
}

HRESULT PlaybackEngine::Stop() {
    m_playing = false;
    m_tickPauseOffset = 0;
    m_tickStart = 0;
    m_currentTime = 0.0;
    m_eof = false;
    return S_OK;
}

HRESULT PlaybackEngine::Seek(double seconds) {
    if (!m_pReader) return MF_E_NOT_INITIALIZED;
    m_currentTime = seconds;
    m_eof = false;

    LONGLONG hnsPosition = (LONGLONG)(seconds * 10000000.0);
    PROPVARIANT var;
    var.vt = VT_I8;
    var.uhVal.QuadPart = hnsPosition;
    HRESULT hr = m_pReader->SetCurrentPosition(GUID_NULL, var);
    if (SUCCEEDED(hr)) {
        m_tickStart = GetTickCount() - (DWORD)(seconds * 1000.0);
        m_tickPauseOffset = 0;
    }
    return hr;
}

HRESULT PlaybackEngine::ReadNextSample() {
    if (!m_pReader) return MF_E_NOT_INITIALIZED;

    IMFSample* pSample = nullptr;
    DWORD dwFlags = 0;
    DWORD streamIndex = 0;
    LONGLONG timestamp = 0;

    while (true) {
        SafeReleasePlayback(pSample);
        HRESULT hr = m_pReader->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0, &streamIndex, &dwFlags, &timestamp, &pSample);

        if (FAILED(hr)) return hr;

        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            m_eof = true;
            SafeReleasePlayback(pSample);
            return S_FALSE;
        }

        if (!pSample) continue;

        if (pSample) {
            if (timestamp >= 0) {
                m_currentTime = (double)timestamp / 10000000.0;
            }
            CopySampleToTexture(pSample);
            SafeReleasePlayback(pSample);
            return S_OK;
        }
    }
}

HRESULT PlaybackEngine::CopySampleToTexture(IMFSample* pSample) {
    if (!pSample || !m_pFrameTexture) return E_INVALIDARG;

    IMFMediaBuffer* pBuffer = nullptr;
    HRESULT hr = pSample->ConvertToContiguousBuffer(&pBuffer);
    if (FAILED(hr)) return hr;

    BYTE* pData = nullptr;
    DWORD cbData = 0;
    hr = pBuffer->Lock(&pData, nullptr, &cbData);
    if (FAILED(hr)) {
        SafeReleasePlayback(pBuffer);
        return hr;
    }

    ID3D11DeviceContext* pContext = nullptr;
    if (m_pRenderer && m_pRenderer->GetDevice()) {
        m_pRenderer->GetDevice()->GetImmediateContext(&pContext);
    }

    if (pContext && m_pFrameTexture) {
        D3D11_TEXTURE2D_DESC desc;
        m_pFrameTexture->GetDesc(&desc);

        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = pContext->Map(m_pFrameTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            UINT srcRowPitch = desc.Width * 4;
            BYTE* pDst = (BYTE*)mapped.pData;
            BYTE* pSrc = pData;
            for (UINT row = 0; row < desc.Height; ++row) {
                memcpy(pDst, pSrc, srcRowPitch < cbData - row * srcRowPitch ? srcRowPitch : cbData - row * srcRowPitch);
                pDst += mapped.RowPitch;
                pSrc += srcRowPitch;
            }
            pContext->Unmap(m_pFrameTexture, 0);
        }
        SafeReleasePlayback(pContext);
    }

    pBuffer->Unlock();
    SafeReleasePlayback(pBuffer);
    return S_OK;
}

HRESULT PlaybackEngine::UpdateFrame() {
    if (!m_playing || m_eof || !m_pReader) return S_OK;

    DWORD elapsed = GetTickCount() - m_tickStart;
    double targetTime = (double)elapsed / 1000.0;

    while (m_currentTime <= targetTime) {
        HRESULT hr = ReadNextSample();
        if (hr == S_FALSE || m_eof) {
            Pause();
            return S_FALSE;
        }
        if (FAILED(hr)) return hr;
    }

    if (m_pFrameTexture && m_pRenderer) {
        RECT dest = { 0, 0, 1920, 1080 };
        m_pRenderer->DrawVideoFrame(m_pFrameTexture, dest);
    }

    return S_OK;
}

} // namespace DirectUI
