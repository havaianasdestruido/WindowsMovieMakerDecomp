/*
 * WLXMP4Parser.cpp
 *
 * Implementation of WLXMP4Parser.dll -- MP4/ISOBMFF container parser for
 * Windows Live Movie Maker 2012.
 *
 * Parses the MP4 box hierarchy to extract track information, chapter data,
 * thumbnails, and structural metadata without decoding media content.
 * Used for fast project loading and file property display.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXMP4Parser.h"
#include "WLXPhotoBase.h"

#include <dshow.h>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cstdio>

// ============================================================================
// Internal structures
// ============================================================================
namespace MP4Parser
{

// Big-endian read helpers (ISOBMFF fields are network byte order)
inline UINT16 ReadBE16(const BYTE* p)
{
    return (UINT16)((p[0] << 8) | p[1]);
}

inline UINT32 ReadBE32(const BYTE* p)
{
    return ((UINT32)p[0] << 24) |
           ((UINT32)p[1] << 16) |
           ((UINT32)p[2] << 8)  |
           ((UINT32)p[3]);
}

inline UINT64 ReadBE64(const BYTE* p)
{
    return ((UINT64)ReadBE32(p) << 32) | ReadBE32(p + 4);
}

// ============================================================================
// Box header (8 or 16 bytes depending on extended size)
// ============================================================================
#pragma pack(push, 1)
struct BoxHeader
{
    UINT32  uSize;          // big-endian; if 1, extended size follows
    UINT32  uType;          // four-character code (big-endian)

    UINT32 GetType() const
    {
        // raw little-endian load -> big-endian fourcc
        return ((uType & 0xFF) << 24) |
               ((uType >> 8) & 0xFF) << 16 |
               ((uType >> 16) & 0xFF) << 8 |
               ((uType >> 24) & 0xFF);
    }

    UINT64 GetSize(const BYTE* extPtr = nullptr) const
    {
        UINT32 sizeBE = uSize;
        UINT32 size = ((sizeBE & 0xFF) << 24) |
                      ((sizeBE >> 8) & 0xFF) << 16 |
                      ((sizeBE >> 16) & 0xFF) << 8 |
                      ((sizeBE >> 24) & 0xFF);
        if (size == 1 && extPtr)
        {
            // extended 64-bit size follows header
            UINT64 extSize = ((UINT64)extPtr[0] << 56) |
                             ((UINT64)extPtr[1] << 48) |
                             ((UINT64)extPtr[2] << 40) |
                             ((UINT64)extPtr[3] << 32) |
                             ((UINT64)extPtr[4] << 24) |
                             ((UINT64)extPtr[5] << 16) |
                             ((UINT64)extPtr[6] << 8)  |
                             ((UINT64)extPtr[7]);
            return extSize;
        }
        return static_cast<UINT64>(size);
    }
};
#pragma pack(pop)

// Box type fourccs (big-endian host values)
enum
{
    kBoxFtyp = 0x66747970,  // 'ftyp'
    kBoxMoov = 0x6D6F6F76,  // 'moov'
    kBoxMvhd = 0x6D766864,  // 'mvhd'
    kBoxTrak = 0x7472616B,  // 'trak'
    kBoxTkhd = 0x746B6864,  // 'tkhd'
    kBoxMdia = 0x6D646961,  // 'mdia'
    kBoxMdhd = 0x6D646864,  // 'mdhd'
    kBoxHdlr = 0x68646C72,  // 'hdlr'
    kBoxMinf = 0x6D696E66,  // 'minf'
    kBoxStbl = 0x7374626C,  // 'stbl'
    kBoxStsd = 0x73747364,  // 'stsd'
    kBoxStts = 0x73747473,  // 'stts'
    kBoxStsc = 0x73747363,  // 'stsc'
    kBoxStsz = 0x7374737A,  // 'stsz'
    kBoxStco = 0x7374636F,  // 'stco'
    kBoxCo64 = 0x636F3634,  // 'co64'
    kBoxMdat = 0x6D646174,  // 'mdat'
    kBoxEdts = 0x65647473,  // 'edts'
    kBoxElst = 0x656C7374,  // 'elst'
    kBoxUdta = 0x75647461,  // 'udta'
    kBoxDinf = 0x64696E66,  // 'dinf'
};

// Handler types ('vide' / 'soun')
enum
{
    kHandlerVideo = 0x76696465,
    kHandlerAudio = 0x736F756E,
};

// ============================================================================
// BoxReader -- walks the child boxes of a container box
// ============================================================================
class BoxReader
{
public:
    BoxReader(const BYTE* pData, UINT64 cbData)
        : m_pData(pData), m_cbData(cbData), m_pos(0)
    {
    }

    // Returns the next child box; false when exhausted or malformed.
    bool Next(UINT32* pType, const BYTE** ppPayload, UINT64* pcbPayload)
    {
        while (m_pos + 8 <= m_cbData)
        {
            UINT64 size = ReadBE32(m_pData + m_pos);
            UINT32 type = ReadBE32(m_pData + m_pos + 4);
            UINT64 headerSize = 8;

            if (size == 1)
            {
                if (m_pos + 16 > m_cbData)
                    break;
                size = ReadBE64(m_pData + m_pos + 8);
                headerSize = 16;
            }
            else if (size == 0)
            {
                size = m_cbData - m_pos;
            }

            if (size < headerSize || m_pos + size > m_cbData)
                break;

            if (pType)
                *pType = type;
            if (ppPayload)
                *ppPayload = m_pData + m_pos + headerSize;
            if (pcbPayload)
                *pcbPayload = size - headerSize;

            m_pos += size;
            return true;
        }
        return false;
    }

private:
    const BYTE* m_pData;
    UINT64      m_cbData;
    UINT64      m_pos;
};

// ============================================================================
// StblBox -- sample table box (stsd, stts, stsc, stsz, stco)
// ============================================================================
class StblBox
{
public:
    StblBox()
        : m_uSampleCount(0)
        , m_uCodecFourCC(0)
        , m_uWidth(0)
        , m_uHeight(0)
        , m_bIsVideo(FALSE)
    {
    }

    HRESULT ParseChildBoxes(const BYTE* pData, DWORD cbData, BOOL bIsVideo)
    {
        if (!pData || cbData == 0)
            return E_INVALIDARG;

        BoxReader reader(pData, cbData);
        UINT32 type = 0;
        const BYTE* pPayload = nullptr;
        UINT64 cbPayload = 0;

        while (reader.Next(&type, &pPayload, &cbPayload))
        {
            switch (type)
            {
            case kBoxStsd:
                ParseStsd(pPayload, cbPayload, bIsVideo);
                break;
            case kBoxStts:
                ParseStts(pPayload, cbPayload);
                break;
            case kBoxStsz:
                ParseStsz(pPayload, cbPayload);
                break;
            default:
                break;
            }
        }

        return S_OK;
    }

    UINT32 GetSampleCount() const { return m_uSampleCount; }
    UINT32 GetCodecFourCC() const { return m_uCodecFourCC; }
    UINT32 GetWidth() const { return m_uWidth; }
    UINT32 GetHeight() const { return m_uHeight; }
    BOOL   IsVideo() const { return m_bIsVideo; }

private:
    // stsd: version/flags(4) + entry_count(4), then sample entries
    void ParseStsd(const BYTE* pData, UINT64 cbData, BOOL bIsVideo)
    {
        if (cbData < 8)
            return;

        UINT32 entryCount = ReadBE32(pData + 4);
        if (entryCount == 0)
            return;

        UINT64 offset = 8;
        if (offset + 8 > cbData)
            return;

        m_uCodecFourCC = ReadBE32(pData + offset + 4);

        if (bIsVideo && offset + 34 <= cbData)
        {
            // VisualSampleEntry: 6 reserved + 2 data_ref_idx (8),
            // then pre_defined(2) + reserved(2) + pre_defined(12) (16),
            // then width(2) + height(2)
            m_uWidth  = ReadBE16(pData + offset + 32);
            m_uHeight = ReadBE16(pData + offset + 34);
        }

        m_bIsVideo = bIsVideo ? TRUE : FALSE;
    }

    // stts: version/flags(4) + entry_count(4), then (sample_count, delta) pairs
    void ParseStts(const BYTE* pData, UINT64 cbData)
    {
        if (cbData < 8)
            return;

        UINT32 entryCount = ReadBE32(pData + 4);
        UINT64 needed = 8 + (UINT64)entryCount * 8;
        if (needed > cbData)
            return;

        UINT64 totalSamples64 = 0;
        UINT64 offset = 8;
        for (UINT32 i = 0; i < entryCount; i++)
        {
            totalSamples64 += ReadBE32(pData + offset);
            offset += 8;
        }

        // Guard against count overflow in a hostile stts box.
        UINT32 totalSamples = (totalSamples64 > 0xFFFFFFFFull)
            ? 0xFFFFFFFFu : (UINT32)totalSamples64;

        if (totalSamples > 0)
            m_uSampleCount = totalSamples;
    }

    // stsz: version/flags(4) + sample_size(4) + sample_count(4)
    void ParseStsz(const BYTE* pData, UINT64 cbData)
    {
        if (cbData < 12)
            return;

        UINT32 sampleSize = ReadBE32(pData + 4);
        UINT32 sampleCount = ReadBE32(pData + 8);

        if (sampleCount > 0)
        {
            m_uSampleCount = sampleCount;
            UNREFERENCED_PARAMETER(sampleSize);
        }
    }

    UINT32  m_uSampleCount;
    UINT32  m_uCodecFourCC;
    UINT32  m_uWidth;
    UINT32  m_uHeight;
    BOOL    m_bIsVideo;
};

// ============================================================================
// TrakBox -- track box (tkhd, mdia, edts)
// ============================================================================
class TrakBox
{
public:
    TrakBox() {}

    HRESULT Parse(BYTE* pData, UINT64 cbData)
    {
        if (!pData || cbData == 0)
            return E_INVALIDARG;

        BoxReader reader(pData, cbData);
        UINT32 type = 0;
        const BYTE* pPayload = nullptr;
        UINT64 cbPayload = 0;

        while (reader.Next(&type, &pPayload, &cbPayload))
        {
            switch (type)
            {
            case kBoxTkhd:
                ParseTkhd(pPayload, cbPayload);
                break;
            case kBoxMdia:
                ParseMdia(pPayload, cbPayload);
                break;
            default:
                break;
            }
        }

        return S_OK;
    }

    MP4TrackInfo& GetTrackInfo() { return m_info; }

private:
    // tkhd: track id, dimensions, duration
    void ParseTkhd(const BYTE* pData, UINT64 cbData)
    {
        if (cbData < 4)
            return;

        BYTE version = pData[0];
        UINT64 offset = 4; // skip version/flags

        if (version == 1)
        {
            if (cbData < 28)
                return;
            m_info.uTrackId = ReadBE32(pData + offset + 16);
            m_info.uDuration = (UINT32)ReadBE64(pData + offset + 24);
            // v1 tkhd: width/height at +88/+92
            if (cbData >= 92)
            {
                m_info.uWidth  = ReadBE32(pData + 88) >> 16;
                m_info.uHeight = ReadBE32(pData + 92) >> 16;
            }
        }
        else
        {
            if (cbData < 20)
                return;
            m_info.uTrackId = ReadBE32(pData + offset + 4);
            m_info.uDuration = ReadBE32(pData + offset + 16);
            // v0 tkhd: width/height at +76/+80
            if (cbData >= 80)
            {
                m_info.uWidth  = ReadBE32(pData + 76) >> 16;
                m_info.uHeight = ReadBE32(pData + 80) >> 16;
            }
        }
    }

    // mdia: mdhd (timescale/duration), hdlr (handler type), minf -> stbl
    void ParseMdia(const BYTE* pData, UINT64 cbData)
    {
        BoxReader reader(pData, cbData);
        UINT32 type = 0;
        const BYTE* pPayload = nullptr;
        UINT64 cbPayload = 0;

        while (reader.Next(&type, &pPayload, &cbPayload))
        {
            switch (type)
            {
            case kBoxMdhd:
                ParseMdhd(pPayload, cbPayload);
                break;
            case kBoxHdlr:
                ParseHdlr(pPayload, cbPayload);
                break;
            case kBoxMinf:
                ParseMinf(pPayload, cbPayload);
                break;
            default:
                break;
            }
        }
    }

    // mdhd: version(1) flags(3) timescale duration
    void ParseMdhd(const BYTE* pData, UINT64 cbData)
    {
        if (cbData < 20)
            return;

        BYTE version = pData[0];
        if (version == 1)
        {
            if (cbData < 28)
                return;
            m_info.uTimeScale = ReadBE32(pData + 20);
            m_info.uDuration  = (UINT32)ReadBE64(pData + 24);
        }
        else
        {
            m_info.uTimeScale = ReadBE32(pData + 12);
            m_info.uDuration  = ReadBE32(pData + 16);
        }
    }

    // hdlr: version/flags(4) pre_defined(4) handler_type(4)
    void ParseHdlr(const BYTE* pData, UINT64 cbData)
    {
        if (cbData < 12)
            return;

        UINT32 handlerType = ReadBE32(pData + 8);
        if (handlerType == kHandlerVideo)
            m_info.bIsVideo = TRUE;
        else if (handlerType == kHandlerAudio)
            m_info.bIsAudio = TRUE;
    }

    // minf -> stbl
    void ParseMinf(const BYTE* pData, UINT64 cbData)
    {
        BoxReader reader(pData, cbData);
        UINT32 type = 0;
        const BYTE* pPayload = nullptr;
        UINT64 cbPayload = 0;

        while (reader.Next(&type, &pPayload, &cbPayload))
        {
            if (type == kBoxStbl)
            {
                m_stbl.ParseChildBoxes(pPayload, (DWORD)cbPayload,
                    m_info.bIsVideo);
                break;
            }
        }

        m_info.uSampleCount = m_stbl.GetSampleCount();
        m_info.uCodecFourCC = m_stbl.GetCodecFourCC();
        if (m_info.bIsVideo)
        {
            if (m_info.uWidth == 0)
                m_info.uWidth = m_stbl.GetWidth();
            if (m_info.uHeight == 0)
                m_info.uHeight = m_stbl.GetHeight();
        }
    }

    MP4TrackInfo    m_info;
    StblBox         m_stbl;
};

// ============================================================================
// MoovBox -- movie box (mvhd, trak[], udta)
// ============================================================================
class MoovBox
{
public:
    MoovBox()
        : m_llDuration(0)
        , m_uTimeScale(1)
    {
    }

    HRESULT Parse(BYTE* pData, UINT64 cbData)
    {
        if (!pData || cbData == 0)
            return E_INVALIDARG;

        BoxReader reader(pData, cbData);
        UINT32 type = 0;
        const BYTE* pPayload = nullptr;
        UINT64 cbPayload = 0;

        while (reader.Next(&type, &pPayload, &cbPayload))
        {
            switch (type)
            {
            case kBoxMvhd:
                ParseMvhd(pPayload, cbPayload);
                break;
            case kBoxTrak:
            {
                // Cap the number of tracks accepted from an untrusted file
                // so a malicious moov cannot cause unbounded allocation.
                if (m_traks.size() >= 1024)
                    break;
                TrakBox trak;
                trak.Parse(const_cast<BYTE*>(pPayload), cbPayload);
                m_traks.push_back(trak);
                break;
            }
            default:
                break;
            }
        }

        return S_OK;
    }

    LONGLONG GetDuration() const
    {
        if (m_llDuration > 0)
            return m_llDuration;
        return 0;
    }

    UINT32 GetTimeScale() const { return m_uTimeScale; }

    UINT32 GetTrackCount() const { return (UINT32)m_traks.size(); }

    HRESULT GetTrackInfo(UINT32 uIndex, MP4TrackInfo* pInfo)
    {
        if (!pInfo)
            return E_INVALIDARG;
        if (uIndex >= m_traks.size())
            return E_INVALIDARG;

        *pInfo = m_traks[uIndex].GetTrackInfo();

        // Compute bitrate from sample count and duration where possible
        if (pInfo->uTimeScale > 0 && pInfo->uDuration > 0 && pInfo->uSampleCount > 0)
        {
            // Rough estimate: assume ~32 KB/sample average for video at 30 fps
            double seconds = (double)pInfo->uDuration / (double)pInfo->uTimeScale;
            if (seconds > 0.0)
            {
                double bits = (double)pInfo->uSampleCount * 32768.0 * 8.0;
                pInfo->uBitrate = (UINT32)(bits / seconds);
            }
        }

        if (pInfo->uTimeScale == 0)
            pInfo->uTimeScale = m_uTimeScale;

        FormatCodecName(pInfo->uCodecFourCC,
            pInfo->wszCodecName,
            sizeof(pInfo->wszCodecName) / sizeof(WCHAR));

        return S_OK;
    }

private:
    // mvhd: version(1) flags(3) timescale duration
    void ParseMvhd(const BYTE* pData, UINT64 cbData)
    {
        if (cbData < 20)
            return;

        BYTE version = pData[0];
        if (version == 1)
        {
            if (cbData < 28)
                return;
            m_uTimeScale = ReadBE32(pData + 20);
            m_llDuration = (LONGLONG)ReadBE64(pData + 24);
        }
        else
        {
            m_uTimeScale = ReadBE32(pData + 12);
            m_llDuration = (LONGLONG)ReadBE32(pData + 16);
        }
    }

    void FormatCodecName(UINT32 fourcc, WCHAR* psz, size_t cch)
    {
        static const struct
        {
            UINT32      uFourCC;
            const WCHAR* wszName;
        } kCodecs[] =
        {
            { 0x61766331, L"H.264 / AVC" },   // 'avc1'
            { 0x61766333, L"H.264 / AVC" },   // 'avc3'
            { 0x68656376, L"H.265 / HEVC" },  // 'hvc1'
            { 0x68657631, L"H.265 / HEVC" },  // 'hev1'
            { 0x6D703476, L"MPEG-4 Visual" }, // 'mp4v'
            { 0x6D703461, L"AAC" },           // 'mp4a'
            { 0x61632D33, L"Dolby Digital" }, // 'ac-3'
            { 0x65632D33, L"E-AC-3" },        // 'ec-3'
            { 0x6D703320, L"MP3" },           // 'mp3 '
            { 0x61766F31, L"AV1" },           // 'av01'
            { 0x76703039, L"VP9" },           // 'vp09'
            { 0x76703830, L"VP8" },           // 'vp80'
            { 0x6A706567, L"Motion JPEG" },  // 'jpeg'
            { 0x4F707573, L"Opus" },          // 'Opus'
            { 0x616C6163, L"ALAC" },          // 'alac'
        };

        for (size_t i = 0; i < sizeof(kCodecs) / sizeof(kCodecs[0]); i++)
        {
            if (kCodecs[i].uFourCC == fourcc)
            {
                wcsncpy_s(psz, cch, kCodecs[i].wszName, _TRUNCATE);
                return;
            }
        }

        // Fallback: raw fourcc as 4 chars
        if (cch >= 5)
        {
            psz[0] = (WCHAR)((fourcc >> 24) & 0xFF);
            psz[1] = (WCHAR)((fourcc >> 16) & 0xFF);
            psz[2] = (WCHAR)((fourcc >> 8) & 0xFF);
            psz[3] = (WCHAR)(fourcc & 0xFF);
            psz[4] = 0;
        }
    }

    LONGLONG                m_llDuration;
    UINT32                  m_uTimeScale;
    std::vector<TrakBox>    m_traks;
};

// ============================================================================
// MP4File -- main parser object
// ============================================================================
class MP4File
{
public:
    MP4File()
        : m_bParsed(false)
        , m_bFastStart(false)
        , m_uBrand(0)
        , m_uMinorVersion(0)
    {
    }

    ~MP4File()
    {
        Close();
    }

    HRESULT Open(LPCWSTR pszFilePath)
    {
        if (!pszFilePath || !pszFilePath[0])
            return E_INVALIDARG;

        m_strFilePath = pszFilePath;

        HANDLE hFile = CreateFileW(pszFilePath, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
            return HRESULT_FROM_WIN32(GetLastError());

        LARGE_INTEGER liSize;
        UINT64 cbFile = 0;
        if (GetFileSizeEx(hFile, &liSize))
            cbFile = (UINT64)liSize.QuadPart;

        if (cbFile < 8)
        {
            CloseHandle(hFile);
            return E_FAIL;
        }

        // Stream the top-level boxes. moov may appear before mdat
        // (fast-start) or after it (non-fast-start).
        HRESULT hr = S_OK;
        UINT64 pos = 0;
        bool bSawMdat = false;
        bool bFoundMoov = false;

        while (pos + 8 <= cbFile)
        {
            LARGE_INTEGER liPos;
            liPos.QuadPart = (LONGLONG)pos;
            if (!SetFilePointerEx(hFile, liPos, NULL, FILE_BEGIN))
                break;

            BYTE header[16];
            DWORD cbRead = 0;
            if (!ReadFile(hFile, header, 8, &cbRead, NULL) || cbRead != 8)
                break;

            UINT32 boxType = ReadBE32(header + 4);
            UINT64 boxSize = ReadBE32(header);
            UINT64 headerSize = 8;

            if (boxSize == 1)
            {
                if (!ReadFile(hFile, header + 8, 8, &cbRead, NULL) || cbRead != 8)
                    break;
                boxSize = ReadBE64(header + 8);
                headerSize = 16;
            }
            else if (boxSize == 0)
            {
                boxSize = cbFile - pos;
            }

            if (boxSize < headerSize || pos + boxSize > cbFile)
                break;

            UINT64 cbPayload = boxSize - headerSize;

            if (boxType == kBoxFtyp && cbPayload >= 8)
            {
                BYTE brand[8];
                if (ReadFile(hFile, brand, 8, &cbRead, NULL) && cbRead == 8)
                {
                    m_uBrand = ReadBE32(brand);
                    m_uMinorVersion = ReadBE32(brand + 4);
                }
            }
            else if (boxType == kBoxMoov)
            {
                // moov is small in practice; guard against malformed files
                if (cbPayload <= 256 * 1024 * 1024)
                {
                    std::vector<BYTE> moovData(static_cast<size_t>(cbPayload));
                    if (ReadFile(hFile, moovData.data(), (DWORD)cbPayload, &cbRead, NULL)
                        && (UINT64)cbRead == cbPayload)
                    {
                        m_moov.reset(new MoovBox());
                        hr = m_moov->Parse(moovData.data(), cbPayload);
                        bFoundMoov = true;
                        m_bFastStart = !bSawMdat;
                    }
                }
            }
            else if (boxType == kBoxMdat)
            {
                bSawMdat = true;
            }

            pos += boxSize;

            if (bFoundMoov && boxType == kBoxMdat)
                break;
        }

        CloseHandle(hFile);

        m_bParsed = (bFoundMoov && m_moov.get() != nullptr);
        return m_bParsed ? S_OK : hr;
    }

    void Close()
    {
        m_moov.reset();
        m_bParsed = false;
    }

    HRESULT GetTrackCount(UINT32* pCount)
    {
        if (!m_bParsed)
            return E_UNEXPECTED;
        if (!pCount)
            return E_INVALIDARG;
        *pCount = m_moov ? m_moov->GetTrackCount() : 0;
        return S_OK;
    }

    HRESULT GetTrackInfo(UINT32 uIndex, MP4TrackInfo* pInfo)
    {
        if (!m_bParsed)
            return E_UNEXPECTED;
        if (!pInfo)
            return E_INVALIDARG;
        if (!m_moov)
            return E_UNEXPECTED;
        return m_moov->GetTrackInfo(uIndex, pInfo);
    }

    HRESULT GetDuration(LONGLONG* pDuration)
    {
        if (!m_bParsed)
            return E_UNEXPECTED;
        if (!pDuration)
            return E_INVALIDARG;
        *pDuration = m_moov ? m_moov->GetDuration() : 0;
        return S_OK;
    }

    HRESULT IsFastStart(BOOL* pFastStart)
    {
        if (!m_bParsed)
            return E_UNEXPECTED;
        if (pFastStart)
            *pFastStart = m_bFastStart ? TRUE : FALSE;
        return S_OK;
    }

    HRESULT GetBrand(UINT32* pBrand, UINT32* pMinorVersion)
    {
        if (!m_bParsed)
            return E_UNEXPECTED;
        if (pBrand)
            *pBrand = m_uBrand;
        if (pMinorVersion)
            *pMinorVersion = m_uMinorVersion;
        return S_OK;
    }

private:
    bool                    m_bParsed;
    bool                    m_bFastStart;
    UINT32                  m_uBrand;
    UINT32                  m_uMinorVersion;
    std::wstring            m_strFilePath;
    std::unique_ptr<MoovBox> m_moov;
};

} // namespace MP4Parser

// ============================================================================
// Exported functions -- DirectShow filter factory functions + COM exports
// ============================================================================

extern "C"
{

WLXMP4P_API HRESULT __stdcall AddMP4SourceFilter(LPCWSTR pszFilePath, IGraphBuilder* pGraph, IBaseFilter** ppFilter)
{
    if (!pszFilePath || !pszFilePath[0] || !pGraph || !ppFilter)
        return E_INVALIDARG;

    *ppFilter = NULL;

    IBaseFilter* pSrc = NULL;
    HRESULT hr = CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, (void**)&pSrc);
    if (FAILED(hr))
        return hr;

    IFileSourceFilter* pFile = NULL;
    hr = pSrc->QueryInterface(IID_IFileSourceFilter, (void**)&pFile);
    if (SUCCEEDED(hr))
    {
        hr = pFile->Load(pszFilePath, NULL);
        pFile->Release();
    }

    if (SUCCEEDED(hr))
        hr = pGraph->AddFilter(pSrc, L"MP4 Source");

    if (SUCCEEDED(hr))
    {
        *ppFilter = pSrc;
        return S_OK;
    }

    pSrc->Release();
    return hr;
}

WLXMP4P_API HRESULT __stdcall BuildMP4FilterGraph(LPCWSTR pszFilePath, IGraphBuilder** ppGraph)
{
    if (!ppGraph)
        return E_INVALIDARG;
    *ppGraph = NULL;

    IGraphBuilder* pGraph = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr))
        return hr;

    IBaseFilter* pSrc = NULL;
    hr = AddMP4SourceFilter(pszFilePath, pGraph, &pSrc);

    if (SUCCEEDED(hr))
    {
        *ppGraph = pGraph;
        return S_OK;
    }

    pGraph->Release();
    return hr;
}

WLXMP4P_API HRESULT __stdcall BuildMP4PlayBack(LPCWSTR pszFilePath, IGraphBuilder* pGraph)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    IGraphBuilder* pLocal = pGraph;
    bool bOwnGraph = false;

    if (!pLocal)
    {
        hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, (void**)&pLocal);
        if (FAILED(hr))
            return hr;
        bOwnGraph = true;
    }

    // Build the full playback graph (source -> demux -> decoders)
    hr = pLocal->RenderFile(pszFilePath, NULL);

    if (bOwnGraph && pLocal)
        pLocal->Release();

    return hr;
}

WLXMP4P_API BOOL __stdcall IsMP4FilePlayable(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return FALSE;

    MP4Parser::MP4File file;
    if (FAILED(file.Open(pszFilePath)))
        return FALSE;

    UINT32 trackCount = 0;
    if (FAILED(file.GetTrackCount(&trackCount)) || trackCount == 0)
        return FALSE;

    // A file is playable if it has at least one track with samples
    for (UINT32 i = 0; i < trackCount; i++)
    {
        MP4Parser::MP4TrackInfo info;
        if (SUCCEEDED(file.GetTrackInfo(i, &info))
            && (info.bIsVideo || info.bIsAudio)
            && info.uSampleCount > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

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
