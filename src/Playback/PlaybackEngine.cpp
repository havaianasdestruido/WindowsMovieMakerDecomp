#include "PlaybackEngine.h"
#include "Renderer.h"
#include <mfapi.h>
#include <mferror.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <propvarutil.h>
#include "FrameCopy.h"

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
    SafeReleasePlayback(m_pUploadTexture);
    SafeReleasePlayback(m_pFrameTexture);
    SafeReleasePlayback(m_pReader);
}

HRESULT PlaybackEngine::OpenFile(const std::wstring& path) {
    Cleanup();
    m_eof = false;
    m_currentTime = 0.0;
    m_duration = 0.0;

    if (!m_pRenderer || !m_pRenderer->GetDevice()) return E_UNEXPECTED;
    using Microsoft::WRL::ComPtr;
    ComPtr<IMFSourceReader> reader;
    ComPtr<ID3D11Texture2D> frame, upload;
    ComPtr<IMFAttributes> attributes;
    HRESULT hr = MFCreateAttributes(attributes.GetAddressOf(), 1);
    if (FAILED(hr)) return hr;
    hr = attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    if (FAILED(hr)) return hr;
    hr = MFCreateSourceReaderFromURL(path.c_str(), attributes.Get(), reader.GetAddressOf());
    if (FAILED(hr)) return hr;

    // Ask the source reader to decode/convert, not to copy compressed native bytes.
    ComPtr<IMFMediaType> requested;
    hr = MFCreateMediaType(requested.GetAddressOf());
    if (FAILED(hr)) return hr;
    hr = requested->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (SUCCEEDED(hr)) hr = requested->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (SUCCEEDED(hr)) hr = reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    if (SUCCEEDED(hr)) hr = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
    if (SUCCEEDED(hr)) hr = reader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, requested.Get());
    if (FAILED(hr)) return hr;

    ComPtr<IMFMediaType> current;
    hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, current.GetAddressOf());
    if (FAILED(hr)) return hr;
    UINT32 width = 0, height = 0, stride = 0;
    hr = MFGetAttributeSize(current.Get(), MF_MT_FRAME_SIZE, &width, &height);
    if (FAILED(hr) || !width || !height) return MF_E_INVALIDMEDIATYPE;
    hr = current->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride);
    if (SUCCEEDED(hr)) m_frameStride = static_cast<LONG>(stride);
    else {
        hr = MFGetStrideForBitmapInfoHeader(MFVideoFormat_RGB32.Data1, width, &m_frameStride);
        if (FAILED(hr)) return hr;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    hr = m_pRenderer->GetDevice()->CreateTexture2D(&desc, nullptr, frame.GetAddressOf());
    if (FAILED(hr)) return hr;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = m_pRenderer->GetDevice()->CreateTexture2D(&desc, nullptr, upload.GetAddressOf());
    if (FAILED(hr)) return hr;

    PROPVARIANT duration;
    PropVariantInit(&duration);
    hr = reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &duration);
    if (SUCCEEDED(hr) && duration.vt == VT_UI8)
        m_duration = static_cast<double>(duration.uhVal.QuadPart) / 10000000.0;
    PropVariantClear(&duration);

    m_pReader = reader.Detach();
    m_pFrameTexture = frame.Detach();
    m_pUploadTexture = upload.Detach();
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

    for (unsigned attempt = 0; attempt < 64; ++attempt) {
        SafeReleasePlayback(pSample);
        HRESULT hr = m_pReader->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0, &streamIndex, &dwFlags, &timestamp, &pSample);

        if (FAILED(hr)) { SafeReleasePlayback(pSample); return hr; }
        if (dwFlags & (MF_SOURCE_READERF_ERROR | MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)) {
            SafeReleasePlayback(pSample);
            return MF_E_INVALIDMEDIATYPE;
        }

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
            hr = CopySampleToTexture(pSample);
            SafeReleasePlayback(pSample);
            return hr;
        }
    }
    return E_FAIL; // A pathological stream must not monopolize the UI thread.
}

HRESULT PlaybackEngine::CopySampleToTexture(IMFSample* pSample) {
    if (!pSample || !m_pFrameTexture || !m_pUploadTexture) return E_INVALIDARG;
    using Microsoft::WRL::ComPtr;
    ComPtr<IMFMediaBuffer> buffer;
    HRESULT hr = pSample->ConvertToContiguousBuffer(buffer.GetAddressOf());
    if (FAILED(hr)) return hr;
    BYTE* data = nullptr;
    DWORD cbData = 0;
    hr = buffer->Lock(&data, nullptr, &cbData);
    if (FAILED(hr)) return hr;

    ComPtr<ID3D11DeviceContext> context;
    if (m_pRenderer && m_pRenderer->GetDevice())
        m_pRenderer->GetDevice()->GetImmediateContext(context.GetAddressOf());
    hr = E_UNEXPECTED;
    if (context) {
        D3D11_TEXTURE2D_DESC desc;
        m_pUploadTexture->GetDesc(&desc);
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        hr = context->Map(m_pUploadTexture, 0, D3D11_MAP_WRITE, 0, &mapped);
        if (SUCCEEDED(hr)) {
            bool copied = CopyRGB32Rows(static_cast<BYTE*>(mapped.pData), mapped.RowPitch,
                data, cbData, desc.Width, desc.Height, m_frameStride);
            context->Unmap(m_pUploadTexture, 0);
            if (copied) context->CopyResource(m_pFrameTexture, m_pUploadTexture);
            else hr = MF_E_BUFFERTOOSMALL;
        }
    }
    HRESULT unlockHr = buffer->Unlock();
    return FAILED(hr) ? hr : unlockHr;
}

HRESULT PlaybackEngine::UpdateFrame() {
    if (!m_playing || m_eof || !m_pReader) return S_OK;

    DWORD elapsed = GetTickCount() - m_tickStart;
    double targetTime = (double)elapsed / 1000.0;

    unsigned frames = 0;
    while (m_currentTime <= targetTime && frames++ < 64) {
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
