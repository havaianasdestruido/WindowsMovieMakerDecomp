#include "pch.h"
// AsyncWorker.cpp - Async worker implementation

#include "AsyncWorker.h"
#include <algorithm>

namespace HMREngine
{

// ============================================================================
// GraphChanged
// ============================================================================
GraphChanged::GraphChanged() = default;
GraphChanged::~GraphChanged() = default;

void GraphChanged::Notify()
{
    m_changed = true;
    if (m_callback) m_callback();
}

bool GraphChanged::HasChanged() const
{
    return m_changed;
}

void GraphChanged::Reset()
{
    m_changed = false;
}

// ============================================================================
// ReloadImage
// ============================================================================
ReloadImage::ReloadImage() = default;
ReloadImage::~ReloadImage() = default;

HRESULT ReloadImage::StartReload(const std::wstring& filePath, ID3D11Device* dev)
{
    if (filePath.empty() || !dev) return E_INVALIDARG;

    m_filePath = filePath;
    m_device = dev;
    m_complete = false;
    m_result = S_OK;

    std::thread worker(&ReloadImage::WorkerThread, this);
    m_workerThread = std::move(worker);

    return S_OK;
}

void ReloadImage::WorkerThread()
{
    m_result = E_FAIL;
    m_complete = true;

    if (m_completeCallback)
        m_completeCallback(m_filePath, m_result);
}

void ReloadImage::Cancel()
{
    m_complete = true;
}

HRESULT ReloadImage::GetTexture(ID3D11Texture2D** ppTex) const
{
    if (!ppTex) return E_POINTER;
    if (!m_texture) return E_FAIL;
    *ppTex = m_texture;
    (*ppTex)->AddRef();
    return S_OK;
}

HRESULT ReloadImage::GetSRV(ID3D11ShaderResourceView** ppSRV) const
{
    if (!ppSRV) return E_POINTER;
    if (!m_srv) return E_FAIL;
    *ppSRV = m_srv;
    (*ppSRV)->AddRef();
    return S_OK;
}

// ============================================================================
// EnumerateNodes
// ============================================================================
EnumerateNodes::EnumerateNodes() = default;
EnumerateNodes::~EnumerateNodes() = default;

void EnumerateNodes::StartEnumeration(X3DChildNode* rootNode)
{
    m_rootNode = rootNode;
    m_nodes.clear();
    m_complete = false;

    if (rootNode)
        EnumerateRecursive(rootNode);

    m_complete = true;
}

void EnumerateNodes::EnumerateRecursive(X3DChildNode* node)
{
    if (!node) return;

    m_nodes.push_back(node);

    for (size_t i = 0; i < node->GetNumChildren(); ++i)
    {
        X3DChildNode* child = node->GetChild(i);
        EnumerateRecursive(child);
    }
}

// ============================================================================
// ImageLoaderWrapper
// ============================================================================
ImageLoaderWrapper::ImageLoaderWrapper() = default;
ImageLoaderWrapper::~ImageLoaderWrapper() = default;

HRESULT ImageLoaderWrapper::LoadImageAsync(const std::wstring& filePath, ID3D11Device* dev)
{
    if (m_cancelled) return E_ABORT;

    m_worker = std::make_unique<ReloadImage>();
    return m_worker->StartReload(filePath, dev);
}

HRESULT ImageLoaderWrapper::LoadImageSync(const std::wstring& filePath, ID3D11Device* dev)
{
    if (filePath.empty() || !dev) return E_INVALIDARG;

    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return HRESULT_FROM_WIN32(GetLastError());

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);
    CloseHandle(hFile);

    return S_OK;
}

bool ImageLoaderWrapper::IsComplete() const
{
    return m_worker ? m_worker->IsComplete() : true;
}

HRESULT ImageLoaderWrapper::GetResult() const
{
    return m_worker ? m_worker->GetResult() : S_OK;
}

HRESULT ImageLoaderWrapper::GetTexture(ID3D11Texture2D** ppTex) const
{
    return m_worker ? m_worker->GetTexture(ppTex) : E_FAIL;
}

HRESULT ImageLoaderWrapper::GetSRV(ID3D11ShaderResourceView** ppSRV) const
{
    return m_worker ? m_worker->GetSRV(ppSRV) : E_FAIL;
}

void ImageLoaderWrapper::Cancel()
{
    m_cancelled = true;
    if (m_worker) m_worker->Cancel();
}

// ============================================================================
// TimeDependentNodesEnumeratorImpl
// ============================================================================
TimeDependentNodesEnumeratorImpl::TimeDependentNodesEnumeratorImpl() = default;
TimeDependentNodesEnumeratorImpl::~TimeDependentNodesEnumeratorImpl() { Clear(); }

void TimeDependentNodesEnumeratorImpl::AddTimeDependentNode(X3DChildNode* node)
{
    if (!node) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_timeDependentNodes.push_back(node);
}

void TimeDependentNodesEnumeratorImpl::RemoveTimeDependentNode(X3DChildNode* node)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find(m_timeDependentNodes.begin(), m_timeDependentNodes.end(), node);
    if (it != m_timeDependentNodes.end())
        m_timeDependentNodes.erase(it);
}

void TimeDependentNodesEnumeratorImpl::Evaluate(double globalTime)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto* node : m_timeDependentNodes)
        EvaluateNode(node, globalTime);
}

void TimeDependentNodesEnumeratorImpl::EvaluateNode(X3DChildNode* node, double time)
{
    if (!node) return;

    auto* timeSensor = dynamic_cast<TimeSensorNode*>(node);
    if (timeSensor)
    {
        timeSensor->Evaluate(time);
    }
}

void TimeDependentNodesEnumeratorImpl::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_timeDependentNodes.clear();
}

UINT TimeDependentNodesEnumeratorImpl::GetNodeCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<UINT>(m_timeDependentNodes.size());
}

X3DChildNode* TimeDependentNodesEnumeratorImpl::GetNode(UINT index) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index >= m_timeDependentNodes.size()) return nullptr;
    return m_timeDependentNodes[index];
}

// ============================================================================
// AsyncWorkerObject
// ============================================================================
AsyncWorkerObject::AsyncWorkerObject() = default;

AsyncWorkerObject::~AsyncWorkerObject()
{
    Cancel();
    if (m_workerThread.joinable())
        m_workerThread.join();
}

void AsyncWorkerObject::Cancel()
{
    if (m_state == WorkerState::Working)
        SetState(WorkerState::Failed);
}

void AsyncWorkerObject::SetState(WorkerState state)
{
    m_state = state;
    if (m_stateChangedCallback)
        m_stateChangedCallback(state);
}

} // namespace HMREngine
