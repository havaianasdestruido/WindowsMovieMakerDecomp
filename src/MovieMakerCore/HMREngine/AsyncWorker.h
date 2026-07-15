#pragma once
// AsyncWorker.h - Async worker objects for HMREngine

#include "HMREngine.h"
#include "X3DNodes.h"
#include <functional>
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

namespace HMREngine
{
    // --- GraphChanged: Notification that the scene graph has changed ---
    class GraphChanged
    {
    public:
        GraphChanged();
        ~GraphChanged();

        void Notify();
        bool HasChanged() const;
        void Reset();

        using Callback = std::function<void()>;
        void SetCallback(Callback cb) { m_callback = cb; }

    protected:
        std::atomic<bool> m_changed{ false };
        Callback m_callback;
    };

    // --- ReloadImage: Async image reload worker ---
    class ReloadImage
    {
    public:
        ReloadImage();
        ~ReloadImage();

        HRESULT StartReload(const std::wstring& filePath, ID3D11Device* dev);
        bool IsComplete() const { return m_complete; }
        HRESULT GetResult() const { return m_result; }

        HRESULT GetTexture(ID3D11Texture2D** ppTex) const;
        HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;

        void Cancel();

        using CompleteCallback = std::function<void(const std::wstring&, HRESULT)>;
        void SetCompleteCallback(CompleteCallback cb) { m_completeCallback = cb; }

    protected:
        std::wstring m_filePath;
        CComPtr<ID3D11Texture2D> m_texture;
        CComPtr<ID3D11ShaderResourceView> m_srv;
        ID3D11Device* m_device = nullptr;
        std::atomic<bool> m_complete{ false };
        HRESULT m_result = S_OK;
        std::thread m_workerThread;
        CompleteCallback m_completeCallback;

        void WorkerThread();
    };

    // --- EnumerateNodes: Async node enumeration ---
    class EnumerateNodes
    {
    public:
        EnumerateNodes();
        ~EnumerateNodes();

        void StartEnumeration(X3DChildNode* rootNode);
        bool IsComplete() const { return m_complete; }

        const std::vector<X3DChildNode*>& GetEnumeratedNodes() const { return m_nodes; }
        UINT GetNodeCount() const { return static_cast<UINT>(m_nodes.size()); }

    protected:
        X3DChildNode* m_rootNode = nullptr;
        std::vector<X3DChildNode*> m_nodes;
        std::atomic<bool> m_complete{ false };

        void EnumerateRecursive(X3DChildNode* node);
    };

    // --- ImageLoaderWrapper: Wrapper for async image loading ---
    class ImageLoaderWrapper
    {
    public:
        ImageLoaderWrapper();
        ~ImageLoaderWrapper();

        HRESULT LoadImageAsync(const std::wstring& filePath, ID3D11Device* dev);
        HRESULT LoadImageSync(const std::wstring& filePath, ID3D11Device* dev);
        bool IsComplete() const;
        HRESULT GetResult() const;

        HRESULT GetTexture(ID3D11Texture2D** ppTex) const;
        HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;

        void Cancel();

    protected:
        std::unique_ptr<ReloadImage> m_worker;
        bool m_cancelled = false;
    };

    // --- TimeDependentNodesEnumeratorImpl: Enumerates time-dependent nodes ---
    class TimeDependentNodesEnumeratorImpl
    {
    public:
        TimeDependentNodesEnumeratorImpl();
        ~TimeDependentNodesEnumeratorImpl();

        void AddTimeDependentNode(X3DChildNode* node);
        void RemoveTimeDependentNode(X3DChildNode* node);

        void Evaluate(double globalTime);
        void Clear();

        UINT GetNodeCount() const;
        X3DChildNode* GetNode(UINT index) const;

    protected:
        std::vector<X3DChildNode*> m_timeDependentNodes;
        mutable std::mutex m_mutex;

        void EvaluateNode(X3DChildNode* node, double time);
    };

    // --- AsyncWorkerObject: Base class for async worker objects ---
    class AsyncWorkerObject
    {
    public:
        enum class WorkerState
        {
            Idle,
            Working,
            Completed,
            Failed,
        };

        AsyncWorkerObject();
        virtual ~AsyncWorkerObject();

        virtual HRESULT Start() = 0;
        virtual void Cancel();

        bool IsIdle() const { return m_state == WorkerState::Idle; }
        bool IsWorking() const { return m_state == WorkerState::Working; }
        bool IsComplete() const { return m_state == WorkerState::Completed || m_state == WorkerState::Failed; }
        bool IsFailed() const { return m_state == WorkerState::Failed; }

        WorkerState GetState() const { return m_state; }
        HRESULT GetResult() const { return m_result; }

        using StateChangedCallback = std::function<void(WorkerState)>;
        void SetStateChangedCallback(StateChangedCallback cb) { m_stateChangedCallback = cb; }

    protected:
        WorkerState m_state = WorkerState::Idle;
        HRESULT m_result = S_OK;
        std::thread m_workerThread;
        StateChangedCallback m_stateChangedCallback;

        void SetState(WorkerState state);
        virtual void OnWorkComplete() {}
    };

} // namespace HMREngine
