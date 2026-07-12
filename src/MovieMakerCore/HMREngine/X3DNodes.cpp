// X3DNodes.cpp - X3D node implementations

#include "X3DNodes.h"
#include <algorithm>
#include <cmath>

namespace HMREngine
{
    // TimeSensor evaluation
    void TimeSensorNode::Evaluate(double globalTime)
    {
        if (!m_enabled) return;

        bool wasActive = m_isActive;

        if (!m_isActive && globalTime >= m_startTime && (m_stopTime <= m_startTime || globalTime < m_stopTime))
        {
            m_isActive = true;
            m_elapsedTime = 0.0;
            if (m_onActiveChanged) m_onActiveChanged(true);
        }
        else if (m_isActive && m_stopTime > m_startTime && globalTime >= m_stopTime)
        {
            m_isActive = false;
            m_fraction = 1.0;
            if (m_onActiveChanged) m_onActiveChanged(false);
            if (m_onFractionChanged) m_onFractionChanged(1.0);
            return;
        }

        if (!m_isActive) return;

        m_elapsedTime = globalTime - m_startTime;

        if (m_cycleInterval > 0.0)
        {
            double cycleTime = fmod(m_elapsedTime, m_cycleInterval);
            m_fraction = cycleTime / m_cycleInterval;
        }
        else
        {
            m_fraction = 0.0;
        }

        if (m_onFractionChanged) m_onFractionChanged(m_fraction);
    }

    // SAIRoute propagation
    void SAIRoute::Propagate()
    {
        if (!m_sourceNode || !m_destNode || !m_enabled) return;

        auto* sourceField = m_sourceNode->GetField(m_sourceField);
        auto* destField = m_destNode->GetField(m_destField);

        if (sourceField && destField)
        {
            destField->CopyFrom(*sourceField);
        }
    }

    // IndexedFaceSet buffer building
    HRESULT IndexedFaceSetNode::BuildVertexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppVB)
    {
        if (!pDev || !m_coord) return E_POINTER;

        if (m_vertexBuffer)
        {
            *ppVB = m_vertexBuffer;
            (*ppVB)->AddRef();
            return S_OK;
        }

        // Simple vertex: position + normal
        struct Vertex
        {
            Vec3 position;
            Vec3 normal;
        };

        size_t vertCount = m_coord->m_point.size();
        std::vector<Vertex> vertices(vertCount);

        for (size_t i = 0; i < vertCount; i++)
        {
            vertices[i].position = m_coord->m_point[i];
            vertices[i].normal = Vec3(0, 1, 0);

            if (m_normal && i < m_normal->m_vector.size())
            {
                vertices[i].normal = m_normal->m_vector[i];
            }
        }

        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = vertices.data();

        HRESULT hr = pDev->CreateBuffer(&bd, &initData, &m_vertexBuffer);
        if (SUCCEEDED(hr))
        {
            m_vertexCount = static_cast<UINT>(vertCount);
            *ppVB = m_vertexBuffer;
            (*ppVB)->AddRef();
        }

        return hr;
    }

    HRESULT IndexedFaceSetNode::BuildIndexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppIB)
    {
        if (!pDev) return E_POINTER;

        if (m_indexBuffer)
        {
            *ppIB = m_indexBuffer;
            (*ppIB)->AddRef();
            return S_OK;
        }

        if (m_coordIndex.m_values.empty()) return E_FAIL;

        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = static_cast<UINT>(m_coordIndex.m_values.size() * sizeof(int));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = m_coordIndex.m_values.data();

        HRESULT hr = pDev->CreateBuffer(&bd, &initData, &m_indexBuffer);
        if (SUCCEEDED(hr))
        {
            m_indexCount = static_cast<UINT>(m_coordIndex.m_values.size());
            *ppIB = m_indexBuffer;
            (*ppIB)->AddRef();
        }

        return hr;
    }

    // Interpolation helpers
    static float LerpFloat(const std::vector<float>& keys, const std::vector<float>& values, float fraction)
    {
        if (keys.empty() || values.empty()) return 0.0f;
        if (keys.size() == 1) return values[0];

        if (fraction <= keys.front()) return values.front();
        if (fraction >= keys.back()) return values.back();

        for (size_t i = 0; i < keys.size() - 1; i++)
        {
            if (fraction >= keys[i] && fraction <= keys[i + 1])
            {
                float t = (fraction - keys[i]) / (keys[i + 1] - keys[i]);
                return values[i] + (values[i + 1] - values[i]) * t;
            }
        }
        return values.back();
    }

    static Vec3 LerpVec3(const std::vector<float>& keys, const std::vector<Vec3>& values, float fraction)
    {
        if (keys.empty() || values.empty()) return Vec3();
        if (keys.size() == 1) return values[0];

        if (fraction <= keys.front()) return values.front();
        if (fraction >= keys.back()) return values.back();

        for (size_t i = 0; i < keys.size() - 1; i++)
        {
            if (fraction >= keys[i] && fraction <= keys[i + 1])
            {
                float t = (fraction - keys[i]) / (keys[i + 1] - keys[i]);
                return values[i] + (values[i + 1] - values[i]) * t;
            }
        }
        return values.back();
    }

    static Rotation4f SlerpRotation(const std::vector<float>& keys, const std::vector<Rotation4f>& values, float fraction)
    {
        if (keys.empty() || values.empty()) return Rotation4f();
        if (keys.size() == 1) return values[0];

        if (fraction <= keys.front()) return values.front();
        if (fraction >= keys.back()) return values.back();

        for (size_t i = 0; i < keys.size() - 1; i++)
        {
            if (fraction >= keys[i] && fraction <= keys[i + 1])
            {
                float t = (fraction - keys[i]) / (keys[i + 1] - keys[i]);
                XMVECTOR q1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&values[i]));
                XMVECTOR q2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&values[i + 1]));
                XMVECTOR result = XMQuaternionSlerp(q1, q2, t);
                Rotation4f r;
                XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&r), result);
                return r;
            }
        }
        return values.back();
    }

    static Rgb LerpColor(const std::vector<float>& keys, const std::vector<Rgb>& values, float fraction)
    {
        if (keys.empty() || values.empty()) return Rgb();
        if (keys.size() == 1) return values[0];

        if (fraction <= keys.front()) return values.front();
        if (fraction >= keys.back()) return values.back();

        for (size_t i = 0; i < keys.size() - 1; i++)
        {
            if (fraction >= keys[i] && fraction <= keys[i + 1])
            {
                float t = (fraction - keys[i]) / (keys[i + 1] - keys[i]);
                return values[i].Lerp(values[i + 1], t);
            }
        }
        return values.back();
    }

    // PositionInterpolatorNode
    Vec3 PositionInterpolatorNode::Interpolate(float fraction) const
    {
        return LerpVec3(m_key.m_value, m_value.m_value, fraction);
    }

    // OrientationInterpolatorNode
    Rotation4f OrientationInterpolatorNode::Interpolate(float fraction) const
    {
        return SlerpRotation(m_key.m_value, m_value.m_value, fraction);
    }

    // ScalarInterpolatorNode
    float ScalarInterpolatorNode::Interpolate(float fraction) const
    {
        return LerpFloat(m_key.m_value, m_value.m_value, fraction);
    }

    // ColorInterpolatorNode
    Rgb ColorInterpolatorNode::Interpolate(float fraction) const
    {
        return LerpColor(m_key.m_value, m_value.m_value, fraction);
    }

    // IntegerSequencerNode
    int IntegerSequencerNode::Evaluate(float fraction) const
    {
        if (m_key.m_value.empty() || m_value.m_value.empty()) return 0;

        if (fraction <= m_key.m_value.front()) return m_value.m_value.front();

        for (size_t i = 0; i < m_key.m_value.size(); i++)
        {
            if (fraction < m_key.m_value[i] && i > 0)
            {
                return m_value.m_value[i - 1];
            }
        }
        return m_value.m_value.back();
    }

    // BooleanSequencerNode
    bool BooleanSequencerNode::Evaluate(float fraction) const
    {
        if (m_key.m_value.empty() || m_value.m_value.empty()) return false;

        if (fraction <= m_key.m_value.front()) return m_value.m_value.front();

        for (size_t i = 0; i < m_key.m_value.size(); i++)
        {
            if (fraction < m_key.m_value[i] && i > 0)
            {
                return m_value.m_value[i - 1];
            }
        }
        return m_value.m_value.back();
    }

    // MovieTextureNode
    void MovieTextureNode::Update(double time)
    {
        if (m_play)
        {
            m_fraction = fmod(time, 1.0);
        }
    }

    // ImageTextureNode
    HRESULT ImageTextureNode::GetTexture(ID3D11Device* pDev, ID3D11ShaderResourceView** ppSRV)
    {
        if (m_srv)
        {
            *ppSRV = m_srv;
            (*ppSRV)->AddRef();
            return S_OK;
        }

        if (m_url.empty()) return E_FAIL;

        HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(
            pDev, m_fullPath.empty() ? m_url.c_str() : m_fullPath.c_str(),
            nullptr, nullptr, ppSRV, nullptr);

        if (SUCCEEDED(hr))
        {
            m_srv = *ppSRV;
            m_loaded = true;
        }

        return hr;
    }

} // namespace HMREngine
