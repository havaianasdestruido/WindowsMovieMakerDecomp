// EncodeProfile.cpp - Encoding profile management implementation

#include "pch.h"
#include "EncodeProfile.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

EncodeProfile::EncodeProfile()
{
}

EncodeProfile::EncodeProfile(const EncodeProfileDesc& desc)
    : m_desc(desc)
{
}

EncodeProfile::~EncodeProfile()
{
}

// ============================================================================
// Identity
// ============================================================================

EncodeProfileId EncodeProfile::GetProfileId() const throw()
{
    return m_desc.profileId;
}

void EncodeProfile::SetProfileId(EncodeProfileId id) throw()
{
    m_desc.profileId = id;
}

ATL::CString EncodeProfile::GetProfileName() const
{
    return m_desc.strProfileName;
}

void EncodeProfile::SetProfileName(LPCWSTR pszName)
{
    m_desc.strProfileName = pszName ? pszName : L"";
}

// ============================================================================
// Video parameters
// ============================================================================

EncodeVideoParams& EncodeProfile::GetVideoParams()
{
    return m_desc.video;
}

const EncodeVideoParams& EncodeProfile::GetVideoParams() const
{
    return m_desc.video;
}

void EncodeProfile::SetVideoWidth(UINT uWidth) throw()
{
    m_desc.video.uWidth = uWidth;
}

UINT EncodeProfile::GetVideoWidth() const throw()
{
    return m_desc.video.uWidth;
}

void EncodeProfile::SetVideoHeight(UINT uHeight) throw()
{
    m_desc.video.uHeight = uHeight;
}

UINT EncodeProfile::GetVideoHeight() const throw()
{
    return m_desc.video.uHeight;
}

void EncodeProfile::SetVideoFrameRate(double dblFps) throw()
{
    m_desc.video.dblFrameRate = dblFps;
}

double EncodeProfile::GetVideoFrameRate() const throw()
{
    return m_desc.video.dblFrameRate;
}

void EncodeProfile::SetVideoBitRate(DWORD dwBitRate) throw()
{
    m_desc.video.dwBitRate = dwBitRate;
}

DWORD EncodeProfile::GetVideoBitRate() const throw()
{
    return m_desc.video.dwBitRate;
}

void EncodeProfile::SetVideoCodec(VideoCodec codec) throw()
{
    m_desc.video.codec = codec;
}

VideoCodec EncodeProfile::GetVideoCodec() const throw()
{
    return m_desc.video.codec;
}

void EncodeProfile::SetH264Profile(UINT uProfile) throw()
{
    m_desc.video.uProfile = uProfile;
}

UINT EncodeProfile::GetH264Profile() const throw()
{
    return m_desc.video.uProfile;
}

void EncodeProfile::SetH264Level(UINT uLevel) throw()
{
    m_desc.video.uLevel = uLevel;
}

UINT EncodeProfile::GetH264Level() const throw()
{
    return m_desc.video.uLevel;
}

void EncodeProfile::SetTwoPassEncoding(bool fTwoPass) throw()
{
    m_desc.video.fTwoPass = fTwoPass;
}

bool EncodeProfile::IsTwoPassEncoding() const throw()
{
    return m_desc.video.fTwoPass;
}

void EncodeProfile::SetHardwareAcceleration(bool fHwAccel) throw()
{
    m_desc.video.fHardwareAcceleration = fHwAccel;
}

bool EncodeProfile::IsHardwareAcceleration() const throw()
{
    return m_desc.video.fHardwareAcceleration;
}

// ============================================================================
// Audio parameters
// ============================================================================

EncodeAudioParams& EncodeProfile::GetAudioParams()
{
    return m_desc.audio;
}

const EncodeAudioParams& EncodeProfile::GetAudioParams() const
{
    return m_desc.audio;
}

void EncodeProfile::SetAudioBitRate(DWORD dwBitRate) throw()
{
    m_desc.audio.dwBitRate = dwBitRate;
}

DWORD EncodeProfile::GetAudioBitRate() const throw()
{
    return m_desc.audio.dwBitRate;
}

void EncodeProfile::SetAudioCodec(AudioCodec codec) throw()
{
    m_desc.audio.codec = codec;
}

AudioCodec EncodeProfile::GetAudioCodec() const throw()
{
    return m_desc.audio.codec;
}

void EncodeProfile::SetAudioSampleRate(DWORD dwSampleRate) throw()
{
    m_desc.audio.dwSampleRate = dwSampleRate;
}

DWORD EncodeProfile::GetAudioSampleRate() const throw()
{
    return m_desc.audio.dwSampleRate;
}

void EncodeProfile::SetAudioChannels(DWORD dwChannels) throw()
{
    m_desc.audio.dwChannels = dwChannels;
}

DWORD EncodeProfile::GetAudioChannels() const throw()
{
    return m_desc.audio.dwChannels;
}

// ============================================================================
// Constraints
// ============================================================================

void EncodeProfile::SetMaxDurationHns(LONGLONG llMax) throw()
{
    m_desc.llMaxDurationHns = llMax;
}

LONGLONG EncodeProfile::GetMaxDurationHns() const throw()
{
    return m_desc.llMaxDurationHns;
}

void EncodeProfile::SetMaxFileSizeBytes(DWORD dwMaxBytes) throw()
{
    m_desc.dwMaxFileSizeBytes = dwMaxBytes;
}

DWORD EncodeProfile::GetMaxFileSizeBytes() const throw()
{
    return m_desc.dwMaxFileSizeBytes;
}

// ============================================================================
// Enabled state
// ============================================================================

void EncodeProfile::SetEnabled(bool fEnabled) throw()
{
    m_desc.fEnabled = fEnabled;
}

bool EncodeProfile::IsEnabled() const throw()
{
    return m_desc.fEnabled;
}

// ============================================================================
// Output format
// ============================================================================

ATL::CString EncodeProfile::GetOutputFileExtension() const
{
    switch (m_desc.video.codec)
    {
    case VideoCodecH264:
    case VideoCodecH265:
        return L".mp4";
    case VideoCodecWMV9:
        return L".wmv";
    case VideoCodecMPEG4:
        return L".avi";
    default:
        return L".mp4";
    }
}

ATL::CString EncodeProfile::GetOutputMimeType() const
{
    switch (m_desc.video.codec)
    {
    case VideoCodecH264:
        return L"video/mp4";
    case VideoCodecWMV9:
        return L"video/x-ms-wmv";
    case VideoCodecMPEG4:
        return L"video/avi";
    default:
        return L"video/mp4";
    }
}

// ============================================================================
// Validation
// ============================================================================

HRESULT EncodeProfile::Validate() const
{
    if (m_desc.video.uWidth == 0 || m_desc.video.uHeight == 0)
        return E_INVALIDARG;

    if (m_desc.video.dblFrameRate <= 0.0 || m_desc.video.dblFrameRate > 120.0)
        return E_INVALIDARG;

    if (m_desc.video.dwBitRate == 0)
        return E_INVALIDARG;

    if (m_desc.audio.dwSampleRate == 0)
        return E_INVALIDARG;

    if (m_desc.audio.dwChannels == 0 || m_desc.audio.dwChannels > 8)
        return E_INVALIDARG;

    return S_OK;
}

// ============================================================================
// Comparison
// ============================================================================

bool EncodeProfile::operator==(const EncodeProfile& other) const
{
    return m_desc.profileId == other.m_desc.profileId &&
           m_desc.video.uWidth == other.m_desc.video.uWidth &&
           m_desc.video.uHeight == other.m_desc.video.uHeight &&
           m_desc.video.dwBitRate == other.m_desc.video.dwBitRate &&
           m_desc.video.codec == other.m_desc.video.codec &&
           m_desc.audio.dwBitRate == other.m_desc.audio.dwBitRate &&
           m_desc.audio.codec == other.m_desc.audio.codec;
}

bool EncodeProfile::operator!=(const EncodeProfile& other) const
{
    return !(*this == other);
}

// ============================================================================
// Static factory methods
// ============================================================================

EncodeProfile EncodeProfile::CreateDefault()
{
    EncodeProfileDesc desc;
    desc.profileId = EncodeProfileDefault;
    desc.strProfileName = L"Default (720p)";
    desc.video.uWidth = 1280;
    desc.video.uHeight = 720;
    desc.video.dblFrameRate = 30.0;
    desc.video.dwBitRate = 5000000;
    desc.video.dwPeakBitRate = 7000000;
    desc.video.codec = VideoCodecH264;
    desc.video.uProfile = 100;
    desc.video.uLevel = 31;
    desc.audio.dwSampleRate = 48000;
    desc.audio.dwChannels = 2;
    desc.audio.dwBitRate = 192000;
    desc.audio.codec = AudioCodecAAC;
    return EncodeProfile(desc);
}

EncodeProfile EncodeProfile::CreateWLP()
{
    EncodeProfileDesc desc;
    desc.profileId = EncodeProfileWLP;
    desc.strProfileName = L"Windows Live Photo (480p)";
    desc.video.uWidth = 640;
    desc.video.uHeight = 480;
    desc.video.dblFrameRate = 30.0;
    desc.video.dwBitRate = 2000000;
    desc.video.dwPeakBitRate = 3000000;
    desc.video.codec = VideoCodecH264;
    desc.video.uProfile = 100;
    desc.video.uLevel = 30;
    desc.audio.dwSampleRate = 44100;
    desc.audio.dwChannels = 2;
    desc.audio.dwBitRate = 128000;
    desc.audio.codec = AudioCodecAAC;
    return EncodeProfile(desc);
}

EncodeProfile EncodeProfile::CreateWLHD()
{
    EncodeProfileDesc desc;
    desc.profileId = EncodeProfileWLHD;
    desc.strProfileName = L"Windows Live HD (1080p)";
    desc.video.uWidth = 1920;
    desc.video.uHeight = 1080;
    desc.video.dblFrameRate = 30.0;
    desc.video.dwBitRate = 8000000;
    desc.video.dwPeakBitRate = 12000000;
    desc.video.codec = VideoCodecH264;
    desc.video.uProfile = 100;
    desc.video.uLevel = 41;
    desc.audio.dwSampleRate = 48000;
    desc.audio.dwChannels = 2;
    desc.audio.dwBitRate = 192000;
    desc.audio.codec = AudioCodecAAC;
    return EncodeProfile(desc);
}

EncodeProfile EncodeProfile::CreateWLX()
{
    EncodeProfileDesc desc;
    desc.profileId = EncodeProfileWLX;
    desc.strProfileName = L"Windows Live Extended (1440p)";
    desc.video.uWidth = 2560;
    desc.video.uHeight = 1440;
    desc.video.dblFrameRate = 30.0;
    desc.video.dwBitRate = 15000000;
    desc.video.dwPeakBitRate = 20000000;
    desc.video.codec = VideoCodecH264;
    desc.video.uProfile = 100;
    desc.video.uLevel = 51;
    desc.audio.dwSampleRate = 48000;
    desc.audio.dwChannels = 2;
    desc.audio.dwBitRate = 256000;
    desc.audio.codec = AudioCodecAAC;
    return EncodeProfile(desc);
}

EncodeProfile EncodeProfile::CreateMP4H264_720p()
{
    EncodeProfileDesc desc;
    desc.profileId = EncodeProfileMP4_H264_720p;
    desc.strProfileName = L"MP4 H.264 720p";
    desc.video.uWidth = 1280;
    desc.video.uHeight = 720;
    desc.video.dblFrameRate = 30.0;
    desc.video.dwBitRate = 5000000;
    desc.video.dwPeakBitRate = 7000000;
    desc.video.codec = VideoCodecH264;
    desc.video.uProfile = 100;
    desc.video.uLevel = 31;
    desc.audio.dwSampleRate = 48000;
    desc.audio.dwChannels = 2;
    desc.audio.dwBitRate = 192000;
    desc.audio.codec = AudioCodecAAC;
    return EncodeProfile(desc);
}

EncodeProfile EncodeProfile::CreateMP4H264_1080p()
{
    EncodeProfileDesc desc;
    desc.profileId = EncodeProfileMP4_H264_1080p;
    desc.strProfileName = L"MP4 H.264 1080p";
    desc.video.uWidth = 1920;
    desc.video.uHeight = 1080;
    desc.video.dblFrameRate = 30.0;
    desc.video.dwBitRate = 8000000;
    desc.video.dwPeakBitRate = 12000000;
    desc.video.codec = VideoCodecH264;
    desc.video.uProfile = 100;
    desc.video.uLevel = 41;
    desc.audio.dwSampleRate = 48000;
    desc.audio.dwChannels = 2;
    desc.audio.dwBitRate = 192000;
    desc.audio.codec = AudioCodecAAC;
    return EncodeProfile(desc);
}

EncodeProfile EncodeProfile::CreateWMV9Standard()
{
    EncodeProfileDesc desc;
    desc.profileId = EncodeProfileWMV9;
    desc.strProfileName = L"WMV9 Standard";
    desc.video.uWidth = 720;
    desc.video.uHeight = 480;
    desc.video.dblFrameRate = 29.97;
    desc.video.dwBitRate = 3000000;
    desc.video.dwPeakBitRate = 5000000;
    desc.video.codec = VideoCodecWMV9;
    desc.video.uProfile = 0;
    desc.video.uLevel = 0;
    desc.audio.dwSampleRate = 44100;
    desc.audio.dwChannels = 2;
    desc.audio.dwBitRate = 128000;
    desc.audio.codec = AudioCodecWMA;
    return EncodeProfile(desc);
}

// ============================================================================
// Static enumeration
// ============================================================================

void EncodeProfile::GetBuiltInProfiles(ATL::CAtlArray<EncodeProfile>& profiles)
{
    profiles.RemoveAll();

    profiles.Add(CreateDefault());
    profiles.Add(CreateWLP());
    profiles.Add(CreateWLHD());
    profiles.Add(CreateWLX());
    profiles.Add(CreateMP4H264_720p());
    profiles.Add(CreateMP4H264_1080p());
    profiles.Add(CreateWMV9Standard());

    AddCommonProfiles(profiles);
}

bool EncodeProfile::FindBuiltInProfile(EncodeProfileId id, EncodeProfile* pProfile)
{
    if (!pProfile)
        return false;

    ATL::CAtlArray<EncodeProfile> profiles;
    GetBuiltInProfiles(profiles);

    for (size_t i = 0; i < profiles.GetCount(); ++i)
    {
        if (profiles[i].GetProfileId() == id)
        {
            *pProfile = profiles[i];
            return true;
        }
    }

    return false;
}

// ============================================================================
// Resolution helpers
// ============================================================================

bool EncodeProfile::IsValidResolution(UINT uWidth, UINT uHeight)
{
    if (uWidth == 0 || uHeight == 0)
        return false;
    if (uWidth > 7680 || uHeight > 4320)
        return false;
    // Must be divisible by 16 for H.264 macroblock alignment
    return (uWidth % 16 == 0) && (uHeight % 16 == 0);
}

double EncodeProfile::GetAspectRatio(UINT uWidth, UINT uHeight)
{
    if (uHeight == 0)
        return 0.0;
    return static_cast<double>(uWidth) / static_cast<double>(uHeight);
}

// ============================================================================
// Private helpers
// ============================================================================

void EncodeProfile::AddCommonProfiles(ATL::CAtlArray<EncodeProfile>& profiles)
{
    // Add any additional platform-specific profiles
    // These may be hardware-dependent (e.g., QuickSync, NVENC presets)
}

} // namespace HMRAVSource
