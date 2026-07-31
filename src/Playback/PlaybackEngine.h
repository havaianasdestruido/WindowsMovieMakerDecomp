#pragma once
#include <windows.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <d3d11.h>
#include <string>

namespace DirectUI {

class GPURenderer;

class __declspec(dllexport) PlaybackEngine {
public:
    PlaybackEngine(GPURenderer* pRenderer);
    ~PlaybackEngine();

    HRESULT OpenFile(const std::wstring& path);
    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();
    HRESULT Seek(double seconds);
    HRESULT UpdateFrame();

    bool IsPlaying() const { return m_playing; }
    double GetCurrentTime() const { return m_currentTime; }
    double GetDuration() const { return m_duration; }
    bool HasMedia() const { return m_pReader != nullptr; }

private:
    void Cleanup();
    HRESULT ReadNextSample();
    HRESULT CopySampleToTexture(IMFSample* pSample);

    GPURenderer* m_pRenderer = nullptr;
    IMFSourceReader* m_pReader = nullptr;
    ID3D11Texture2D* m_pFrameTexture = nullptr;
    bool m_playing = false;
    bool m_eof = false;
    double m_currentTime = 0.0;
    double m_duration = 0.0;
    DWORD m_tickStart = 0;
    DWORD m_tickPauseOffset = 0;
};

} // namespace DirectUI
