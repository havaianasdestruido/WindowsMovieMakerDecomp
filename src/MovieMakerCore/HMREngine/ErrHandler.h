#pragma once
// ErrHandler.h - HMREngine::ErrHandler

#include "HMREngine.h"
#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace HMREngine
{
    struct ErrorInfo
    {
        HMRError code;
        std::string message;
        std::string file;
        int line = 0;
        double timestamp = 0.0;
    };

    class ErrHandler
    {
    public:
        ErrHandler();
        ~ErrHandler();

        void HandleError(HMRError code, const std::string& msg, const char* file = "", int line = 0);
        void HandleError(const std::string& msg);
        void HandleHRESULT(HRESULT hr, const std::string& context);

        size_t GetErrorCount() const { return m_errors.size(); }
        const ErrorInfo& GetError(size_t index) const { return m_errors[index]; }
        void ClearErrors() { m_errors.clear(); }

        bool HasErrors() const { return !m_errors.empty(); }

        // Logging
        void LogMessage(const std::string& msg);
        void LogWarning(const std::string& msg);
        void LogError(const std::string& msg);

        enum LogLevel { LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG };
        void SetLogLevel(LogLevel level) { m_logLevel = level; }
        LogLevel GetLogLevel() const { return m_logLevel; }

        using LogCallback = std::function<void(LogLevel, const std::string&)>;
        void SetLogCallback(LogCallback cb) { m_logCallback = cb; }

        // D3D error helper
        static std::string HResultToString(HRESULT hr);

        // User-friendly error descriptions
        static std::string GetErrorDescription(HMRError code);
        static bool IsCriticalError(HMRError code);

    private:
        std::vector<ErrorInfo> m_errors;
        mutable std::mutex m_mutex;
        LogLevel m_logLevel = LOG_INFO;
        LogCallback m_logCallback;
    };

    #define HMR_ERR(handler, code, msg) (handler)->HandleError(code, msg, __FILE__, __LINE__)
    #define HMR_LOG(handler, msg) (handler)->LogMessage(msg)
    #define HMR_WARN(handler, msg) (handler)->LogWarning(msg)
    #define HMR_D3D_ERR(handler, hr, ctx) (handler)->HandleHRESULT(hr, ctx)

} // namespace HMREngine
