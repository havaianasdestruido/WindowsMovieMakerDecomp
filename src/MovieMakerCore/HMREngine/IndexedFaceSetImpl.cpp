#include "pch.h"

// IndexedFaceSetImpl.cpp - IndexedFaceSet implementations

#include "IndexedFaceSetImpl.h"

namespace HMREngine
{
    IndexedFaceSetImpl::IndexedFaceSetImpl() {}
    IndexedFaceSetImpl::~IndexedFaceSetImpl() { Shutdown(); }

    HRESULT IndexedFaceSetImpl::Initialize(IndexedFaceSetNode* node, ID3D11Device* dev)
    {
        if (!node || !dev) return E_POINTER;
        m_node = node;
        m_ccw = node->m_ccw;
        m_solid = node->m_solid;

        if (node->m_coord) m_vertices.resize(node->m_coord->m_point.size());
        return RebuildBuffers(dev);
    }

    void IndexedFaceSetImpl::Shutdown()
    {
        m_vertexBuffer.Release();
        m_indexBuffer.Release();
        m_vertices.clear();
        m_indices.clear();
    }

    HRESULT IndexedFaceSetImpl::RebuildBuffers(ID3D11Device* dev)
    {
        if (!dev || !m_node) return E_POINTER;

        m_vertices.clear();
        m_indices.clear();

        // Build vertex data from coordinate node
        if (m_node->m_coord)
        {
            size_t vertCount = m_node->m_coord->m_point.size();
            m_vertices.resize(vertCount);

            for (size_t i = 0; i < vertCount; i++)
            {
                m_vertices[i].position = m_node->m_coord->m_point[i];
                m_vertices[i].normal = Vec3(0, 1, 0);
                m_vertices[i].texCoord = Vec2(0, 0);
                m_vertices[i].color = Rgb(1, 1, 1);
            }

            // Apply normals if available
            if (m_node->m_normal)
            {
                for (size_t i = 0; i < vertCount && i < m_node->m_normal->m_vector.size(); i++)
                {
                    m_vertices[i].normal = m_node->m_normal->m_vector[i];
                }
            }

            // Apply texture coordinates if available
            if (m_node->m_texCoord)
            {
                for (size_t i = 0; i < vertCount && i < m_node->m_texCoord->m_point.size(); i++)
                {
                    m_vertices[i].texCoord = m_node->m_texCoord->m_point[i];
                }
            }

            // Apply vertex colors if available
            if (m_node->m_color)
            {
                for (size_t i = 0; i < vertCount && i < m_node->m_color->m_color.size(); i++)
                {
                    m_vertices[i].color = m_node->m_color->m_color[i];
                }
            }
        }

        // Build index buffer from coordIndex
        // Convert -1 delimited face lists to triangle indices
        if (!m_node->m_coordIndex.m_values.empty())
        {
            std::vector<int> currentFace;

            for (int idx : m_node->m_coordIndex.m_values)
            {
                if (idx == -1)
                {
                    if (currentFace.size() >= 3)
                    {
                        TriangulateFace(currentFace, m_indices);
                    }
                    currentFace.clear();
                }
                else
                {
                    currentFace.push_back(idx);
                }
            }

            if (currentFace.size() >= 3)
            {
                TriangulateFace(currentFace, m_indices);
            }
        }

        m_vertexCount = static_cast<UINT>(m_vertices.size());
        m_indexCount = static_cast<UINT>(m_indices.size());

        // Create vertex buffer
        if (!m_vertices.empty())
        {
            D3D11_BUFFER_DESC vbd{};
            vbd.ByteWidth = m_vertexCount * sizeof(VertexAttributes);
            vbd.Usage = D3D11_USAGE_DEFAULT;
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA vInit{};
            vInit.pSysMem = m_vertices.data();

            HRESULT hr = dev->CreateBuffer(&vbd, &vInit, &m_vertexBuffer);
            if (FAILED(hr)) return hr;
        }

        // Create index buffer
        if (!m_indices.empty())
        {
            D3D11_BUFFER_DESC ibd{};
            ibd.ByteWidth = m_indexCount * sizeof(UINT);
            ibd.Usage = D3D11_USAGE_DEFAULT;
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA iInit{};
            iInit.pSysMem = m_indices.data();

            HRESULT hr = dev->CreateBuffer(&ibd, &iInit, &m_indexBuffer);
            if (FAILED(hr)) return hr;
        }

        ComputeBoundingBox();
        return S_OK;
    }

    void IndexedFaceSetImpl::TriangulateFace(const std::vector<int>& faceIndices, std::vector<UINT>& outIndices)
    {
        // Fan triangulation for convex polygons
        for (size_t i = 1; i + 1 < faceIndices.size(); i++)
        {
            if (m_ccw)
            {
                outIndices.push_back(static_cast<UINT>(faceIndices[0]));
                outIndices.push_back(static_cast<UINT>(faceIndices[i]));
                outIndices.push_back(static_cast<UINT>(faceIndices[i + 1]));
            }
            else
            {
                outIndices.push_back(static_cast<UINT>(faceIndices[0]));
                outIndices.push_back(static_cast<UINT>(faceIndices[i + 1]));
                outIndices.push_back(static_cast<UINT>(faceIndices[i]));
            }
        }
    }

    Vec3 IndexedFaceSetImpl::ComputeFaceNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2)
    {
        Vec3 e1 = v1 - v0;
        Vec3 e2 = v2 - v0;
        return e1.Cross(e2).Normalized();
    }

    void IndexedFaceSetImpl::ComputeBoundingBox()
    {
        if (m_vertices.empty())
        {
            m_bboxMin = Vec3();
            m_bboxMax = Vec3();
            return;
        }

        m_bboxMin = m_vertices[0].position;
        m_bboxMax = m_vertices[0].position;

        for (size_t i = 1; i < m_vertices.size(); i++)
        {
            const Vec3& p = m_vertices[i].position;
            m_bboxMin.x = (std::min)(m_bboxMin.x, p.x);
            m_bboxMin.y = (std::min)(m_bboxMin.y, p.y);
            m_bboxMin.z = (std::min)(m_bboxMin.z, p.z);
            m_bboxMax.x = (std::max)(m_bboxMax.x, p.x);
            m_bboxMax.y = (std::max)(m_bboxMax.y, p.y);
            m_bboxMax.z = (std::max)(m_bboxMax.z, p.z);
        }
    }

} // namespace HMREngine
