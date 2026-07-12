#pragma once
// ReloadImageWorker.h - Image reload thread pool worker

#include "HMREngine.h"
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace HMREngine
{
    class ResourceCache;

    struct ImageReloadJob
    {
        std::string filePath;
        std::function<void(const std::string&, bool)> onComplete;
        bool completed = false;
        bool success = false;
    };

    class ReloadImageWorker
    {
    public:
        ReloadImageWorker();
        ~ReloadImageWorker();

        void Initialize(ResourceCache* cache, int threadCount = 1);
        void Shutdown();

        void SubmitJob(const std::string& filePath,
            std::function<void(const std::string&, bool)> onComplete = nullptr);
        void SubmitJobs(const std::vector<std::string>& filePaths);

        bool IsIdle() const;
        size_t GetPendingJobCount() const;
        size_t GetCompletedJobCount() const { return m_completedCount; }

        void CancelAllJobs();
        void WaitUntilIdle();

    private:
        ResourceCache* m_cache = nullptr;
        std::vector<std::thread> m_workers;
        std::queue<ImageReloadJob> m_jobQueue;

        mutable std::mutex m_mutex;
        std::condition_variable m_jobAvailable;
        std::condition_variable m_allDone;

        std::atomic<bool> m_shutdown{ false };
        std::atomic<size_t> m_pendingCount{ 0 };
        std::atomic<size_t> m_completedCount{ 0 };

        void WorkerThread();
        void ProcessJob(ImageReloadJob& job);
    };

} // namespace HMREngine
