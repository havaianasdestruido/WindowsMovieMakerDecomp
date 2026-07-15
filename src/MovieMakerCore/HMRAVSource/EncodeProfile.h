/*
 * EncodeProfile.h
 *
 * EncodeProfile - Encoding profile management for video export.
 * Defines supported resolutions, bitrates, codecs, and quality settings
 * used by the transcode pipeline.
 *
 * Profiles include presets for standard definition, high definition,
 * and device-specific output formats.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef ENCODEPROFILE_H
#define ENCODEPROFILE_H

#include "HMRAVSourceTypes.h"

namespace HMRAVSource
{

// ============================================================================
// EncodeVideoParams
// ============================================================================
struct EncodeVideoParams
{
    UINT        uWidth = 1920;
    UINT        uHeight = 1080;
    double      dblFrameRate = 30.0;
    DWORD       dwBitRate = 8000000;        // 8 Mbps
    DWORD       dwPeakBitRate = 10000000;   // 10 Mbps
    VideoCodec  codec = VideoCodecH264;
    UINT        uProfile = 100;              // H.264 High profile
    UINT        uLevel = 41;                 // H.264 Level 4.1
    bool        fTwoPass = false;
    bool        fHardwareAcceleration = true;
};

// ============================================================================
// EncodeAudioParams
// ============================================================================
struct EncodeAudioParams
{
    DWORD       dwSampleRate = 48000;
    DWORD       dwChannels = 2;
    DWORD       dwBitsPerSample = 16;
    DWORD       dwBitRate = 192000;          // 192 kbps
    AudioCodec  codec = AudioCodecAAC;
    DWORD       dwBlockAlign = 0;
};

// ============================================================================
// EncodeProfileDesc
// ============================================================================
struct EncodeProfileDesc
{
    EncodeProfileId     profileId = EncodeProfileDefault;
    ATL::CString        strProfileName;
    EncodeVideoParams   video;
    EncodeAudioParams   audio;
    LONGLONG            llMaxDurationHns = 0;  // 0 = unlimited
    DWORD               dwMaxFileSizeBytes = 0;
    bool                fEnabled = true;
};

// ============================================================================
// EncodeProfile
// ============================================================================
// Manages a single encoding profile and provides static methods to enumerate
// built-in profiles. Each profile defines the video/audio encoding parameters
// for a specific output format or quality level.
//
class AVSOURCE_API EncodeProfile
{
public:
    EncodeProfile();
    EncodeProfile(const EncodeProfileDesc& desc);
    ~EncodeProfile();

    // Identity
    EncodeProfileId GetProfileId() const throw();
    void SetProfileId(EncodeProfileId id) throw();

    ATL::CString GetProfileName() const;
    void SetProfileName(LPCWSTR pszName);

    // Video parameters
    EncodeVideoParams& GetVideoParams();
    const EncodeVideoParams& GetVideoParams() const;

    void SetVideoWidth(UINT uWidth) throw();
    UINT GetVideoWidth() const throw();

    void SetVideoHeight(UINT uHeight) throw();
    UINT GetVideoHeight() const throw();

    void SetVideoFrameRate(double dblFps) throw();
    double GetVideoFrameRate() const throw();

    void SetVideoBitRate(DWORD dwBitRate) throw();
    DWORD GetVideoBitRate() const throw();

    void SetVideoCodec(VideoCodec codec) throw();
    VideoCodec GetVideoCodec() const throw();

    void SetH264Profile(UINT uProfile) throw();
    UINT GetH264Profile() const throw();

    void SetH264Level(UINT uLevel) throw();
    UINT GetH264Level() const throw();

    void SetTwoPassEncoding(bool fTwoPass) throw();
    bool IsTwoPassEncoding() const throw();

    void SetHardwareAcceleration(bool fHwAccel) throw();
    bool IsHardwareAcceleration() const throw();

    // Audio parameters
    EncodeAudioParams& GetAudioParams();
    const EncodeAudioParams& GetAudioParams() const;

    void SetAudioBitRate(DWORD dwBitRate) throw();
    DWORD GetAudioBitRate() const throw();

    void SetAudioCodec(AudioCodec codec) throw();
    AudioCodec GetAudioCodec() const throw();

    void SetAudioSampleRate(DWORD dwSampleRate) throw();
    DWORD GetAudioSampleRate() const throw();

    void SetAudioChannels(DWORD dwChannels) throw();
    DWORD GetAudioChannels() const throw();

    // Constraints
    void SetMaxDurationHns(LONGLONG llMax) throw();
    LONGLONG GetMaxDurationHns() const throw();

    void SetMaxFileSizeBytes(DWORD dwMaxBytes) throw();
    DWORD GetMaxFileSizeBytes() const throw();

    // Enabled state
    void SetEnabled(bool fEnabled) throw();
    bool IsEnabled() const throw();

    // Output format
    ATL::CString GetOutputFileExtension() const;
    ATL::CString GetOutputMimeType() const;

    // Validation
    HRESULT Validate() const;

    // Comparison
    bool operator==(const EncodeProfile& other) const;
    bool operator!=(const EncodeProfile& other) const;

    // Static factory - built-in profiles
    static EncodeProfile CreateDefault();
    static EncodeProfile CreateWLP();          // Windows Live Photo standard
    static EncodeProfile CreateWLHD();         // Windows Live HD
    static EncodeProfile CreateWLX();          // Windows Live extended
    static EncodeProfile CreateMP4H264_720p();
    static EncodeProfile CreateMP4H264_1080p();
    static EncodeProfile CreateWMV9Standard();

    // Enumerate all built-in profiles
    static void GetBuiltInProfiles(ATL::CAtlArray<EncodeProfile>& profiles);

    // Find profile by ID
    static bool FindBuiltInProfile(EncodeProfileId id, EncodeProfile* pProfile);

    // Resolution helpers
    static bool IsValidResolution(UINT uWidth, UINT uHeight);
    static double GetAspectRatio(UINT uWidth, UINT uHeight);

private:
    EncodeProfileDesc m_desc;

    static void AddCommonProfiles(ATL::CAtlArray<EncodeProfile>& profiles);
};

} // namespace HMRAVSource

#endif // ENCODEPROFILE_H
