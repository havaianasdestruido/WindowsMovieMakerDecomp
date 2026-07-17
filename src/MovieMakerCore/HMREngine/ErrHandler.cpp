#include "pch.h"

// ErrHandler.cpp - HMREngine::ErrHandler implementation

#include "ErrHandler.h"
#include <chrono>

namespace HMREngine
{
    ErrHandler::ErrHandler()
    {
    }

    ErrHandler::~ErrHandler()
    {
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
