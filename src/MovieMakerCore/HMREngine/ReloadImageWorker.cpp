#include "pch.h"

// ReloadImageWorker.cpp - Image reload thread pool worker implementation

#include "ReloadImageWorker.h"
#include "ResourceCache.h"

namespace HMREngine
{
    ReloadImageWorker::ReloadImageWorker()
    {
    }

    ReloadImageWorker::~ReloadImageWorker()
    {
        Shutdown();
    }

    void ReloadImageWorker::Initialize(ResourceCache* cache, int threadCount)
    {
        m_cache = cache;
        m_shutdown = false;

        for (int i = 0; i < threadCount; i++)
        {
            m_workers.emplace_back(&ReloadImageWorker::WorkerThread, this);
        }
    }

    void ReloadImageWorker::Shutdown()
    {
        m_shutdown = true;
        m_jobAvailable.notify_all();

        for (auto& t : m_workers)
        {
            if (t.joinable()) t.join();
        }
        m_workers.clear();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            while (!m_jobQueue.empty()) m_jobQueue.pop();
        }
    }

    void ReloadImageWorker::SubmitJob(const std::string& filePath,
        std::function<void(const std::string&, bool)> onComplete)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            ImageReloadJob job;
            job.filePath = filePath;
            job.onComplete = onComplete;
            m_jobQueue.push(std::move(job));
            m_pendingCount++;
        }
        m_jobAvailable.notify_one();
    }

    void ReloadImageWorker::SubmitJobs(const std::vector<std::string>& filePaths)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& path : filePaths)
            {
                ImageReloadJob job;
                job.filePath = path;
                m_jobQueue.push(std::move(job));
                m_pendingCount++;
            }
        }
        m_jobAvailable.notify_all();
    }

    void ReloadImageWorker::WorkerThread()
    {
        while (!m_shutdown)
        {
            ImageReloadJob job;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_jobAvailable.wait(lock, [this] { return m_shutdown || !m_jobQueue.empty(); });

                if (m_shutdown && m_jobQueue.empty()) return;

                if (!m_jobQueue.empty())
                {
                    job = std::move(m_jobQueue.front());
                    m_jobQueue.pop();
                }
            }

            if (!job.filePath.empty())
            {
                ProcessJob(job);
            }
        }
    }

    void ReloadImageWorker::ProcessJob(ImageReloadJob& job)
    {
        if (m_cache)
        {
            m_cache->ReloadTexture(job.filePath);
            job.success = m_cache->GetTexture(job.filePath) != nullptr;
        }
        else
        {
            job.success = false;
        }

        job.completed = true;
        m_pendingCount--;
        m_completedCount++;

        if (job.onComplete)
        {
            job.onComplete(job.filePath, job.success);
        }

        m_allDone.notify_all();
    }

    bool ReloadImageWorker::IsIdle() const
    {
        return m_pendingCount == 0;
    }

    size_t ReloadImageWorker::GetPendingJobCount() const
    {
        return m_pendingCount;
    }

    void ReloadImageWorker::CancelAllJobs()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_jobQueue.empty()) m_jobQueue.pop();
        m_pendingCount = 0;
    }

    void ReloadImageWorker::WaitUntilIdle()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_allDone.wait(lock, [this] { return m_pendingCount == 0; });
    }

} // namespace HMREngine
