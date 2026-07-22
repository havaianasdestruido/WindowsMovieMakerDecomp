#include "pch.h"

// ErrHandler.cpp - HMREngine::ErrHandler implementation

#include "ErrHandler.h"
#include <chrono>

// HMREngine.h undefs SDK macros to define its own HMRError enum, then
// re-defines them. That means after including the header, E_FAIL etc. are
// macros again. We need to undef them so HMRError::E_FAIL works in switch.
#undef S_OK
#undef E_FAIL
#undef E_OUTOFMEMORY
#undef E_INVALIDARG
#undef E_NOTIMPL
#undef E_POINTER
#undef E_UNEXPECTED
#undef E_ACCESSDENIED
#undef E_ABORT
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_RESET
#undef D3DERR_DEVICELOST
#undef D3DERR_DEVICENOTRESET

namespace HMREngine
{
    ErrHandler::ErrHandler()
    {
    }

    ErrHandler::~ErrHandler()
    {
    }

    std::string ErrHandler::GetErrorDescription(HMRError code)
    {
        switch (code)
        {
        case HMRError::S_OK:                  return "Success";
        case HMRError::E_FAIL:                return "General failure";
        case HMRError::E_OUTOFMEMORY:         return "Not enough memory";
        case HMRError::E_INVALIDARG:          return "Invalid argument";
        case HMRError::E_NOTIMPL:             return "Not implemented";
        case HMRError::E_POINTER:             return "Invalid pointer";
        case HMRError::E_UNEXPECTED:          return "Unexpected error";
        case HMRError::E_ACCESSDENIED:        return "Access denied";
        case HMRError::E_ABORT:               return "Operation aborted";
        case HMRError::DXGI_ERROR_DEVICE_REMOVED: return "Graphics device removed";
        case HMRError::DXGI_ERROR_DEVICE_RESET:   return "Graphics device reset";
        case HMRError::D3DERR_DEVICELOST:     return "3D device lost";
        case HMRError::D3DERR_DEVICENOTRESET: return "3D device not reset";
        default:                              return "Unknown error";
        }
    }

    bool ErrHandler::IsCriticalError(HMRError code)
    {
        switch (code)
        {
        case HMRError::E_OUTOFMEMORY:
        case HMRError::E_POINTER:
        case HMRError::E_ACCESSDENIED:
        case HMRError::DXGI_ERROR_DEVICE_REMOVED:
        case HMRError::D3DERR_DEVICELOST:
            return true;
        default:
            return false;
        }
    }

    void ErrHandler::HandleError(HMRError code, const std::string& msg, const char* file, int line)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        ErrorInfo info;
        info.code = code;
        info.message = msg;
        info.file = file ? file : "";
        info.line = line;

        auto now = std::chrono::steady_clock::now();
        info.timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();

        m_errors.push_back(info);

        if (m_logCallback)
        {
            m_logCallback(LOG_ERROR, "[" + HResultToString(static_cast<HRESULT>(code)) + "] " + msg);
        }

        std::string desc = GetErrorDescription(code);
        std::string body = desc + "\n\n" + msg;
        if (!info.file.empty())
        {
            body += "\n\nLocation: " + info.file;
            if (info.line > 0)
            {
                char lineBuf[32];
                sprintf_s(lineBuf, " line %d", info.line);
                body += lineBuf;
            }
        }

        UINT mbIcon = IsCriticalError(code) ? MB_ICONERROR : MB_ICONWARNING;

        // Convert body to wide string for MessageBoxW
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, body.c_str(), -1, NULL, 0);
        std::vector<wchar_t> wideBody(wideLen);
        MultiByteToWideChar(CP_UTF8, 0, body.c_str(), -1, wideBody.data(), wideLen);

        ::MessageBoxW(NULL, wideBody.data(), SUNDANCE_PRODUCT_NAME, MB_OK | mbIcon | MB_TASKMODAL);
    }

    void ErrHandler::HandleError(const std::string& msg)
    {
        HandleError(static_cast<HMRError>(1), msg, "", 0);
    }

    void ErrHandler::HandleHRESULT(HRESULT hr, const std::string& context)
    {
        std::string msg = context + ": " + HResultToString(hr);
        HandleError(static_cast<HMRError>(hr), msg, "", 0);
    }

    void ErrHandler::LogMessage(const std::string& msg)
    {
        if (m_logLevel <= LOG_INFO && m_logCallback) m_logCallback(LOG_INFO, msg);
    }

    void ErrHandler::LogWarning(const std::string& msg)
    {
        if (m_logLevel <= LOG_WARNING && m_logCallback) m_logCallback(LOG_WARNING, msg);
    }

    void ErrHandler::LogError(const std::string& msg)
    {
        if (m_logLevel <= LOG_ERROR && m_logCallback) m_logCallback(LOG_ERROR, msg);
    }

    std::string ErrHandler::HResultToString(HRESULT hr)
    {
        char buf[64];
        sprintf_s(buf, "0x%08X", static_cast<unsigned int>(hr));
        return std::string(buf);
    }

} // namespace HMREngine

// Restore Windows SDK macros
#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif
#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#endif
#ifndef E_INVALIDARG
#define E_INVALIDARG ((HRESULT)0x80070057L)
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL ((HRESULT)0x80004001L)
#endif
#ifndef E_POINTER
#define E_POINTER ((HRESULT)0x80004003L)
#endif
#ifndef E_UNEXPECTED
#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)
#endif
#ifndef E_ACCESSDENIED
#define E_ACCESSDENIED ((HRESULT)0x80070005L)
#endif
#ifndef E_ABORT
#define E_ABORT ((HRESULT)0x40001L)
#endif
#ifndef DXGI_ERROR_DEVICE_REMOVED
#define DXGI_ERROR_DEVICE_REMOVED ((HRESULT)0x887A0005L)
#endif
#ifndef DXGI_ERROR_DEVICE_RESET
#define DXGI_ERROR_DEVICE_RESET ((HRESULT)0x887A0006L)
#endif
#ifndef D3DERR_DEVICELOST
#define D3DERR_DEVICELOST ((HRESULT)0x887601C2L)
#endif
#ifndef D3DERR_DEVICENOTRESET
#define D3DERR_DEVICENOTRESET ((HRESULT)0x887601CCL)
#endif
