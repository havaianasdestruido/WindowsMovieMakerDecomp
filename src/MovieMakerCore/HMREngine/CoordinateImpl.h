#pragma once
// CoordinateImpl.h - Coordinate, Normal, Color, ColorRGBA, TextureCoordinate

#include "HMREngine.h"
#include "X3DNodes.h"
#include "X3DMath.h"
#include <vector>
#include <d3d11.h>
#include <atlbase.h>

namespace HMREngine
{
    class CoordinateImpl : public ImplObject<CoordinateImpl>
    {
    public:
        HRESULT Initialize(CoordinateNode* node);
        void Shutdown();

        const std::vector<Vec3>& GetPoints() const { return m_points; }
        Vec3 GetPoint(size_t index) const;
        size_t GetCount() const { return m_points.size(); }

    private:
        std::vector<Vec3> m_points;
    };

    class NormalImpl : public ImplObject<NormalImpl>
    {
    public:
        HRESULT Initialize(NormalNode* node);
        void Shutdown();

        const std::vector<Vec3>& GetVectors() const { return m_vectors; }
        Vec3 GetVector(size_t index) const;
        size_t GetCount() const { return m_vectors.size(); }

    private:
        std::vector<Vec3> m_vectors;
    };

    class ColorImpl : public ImplObject<ColorImpl>
    {
    public:
        HRESULT Initialize(ColorNode* node);
        void Shutdown();

        const std::vector<Rgb>& GetColors() const { return m_colors; }
        Rgb GetColor(size_t index) const;
        size_t GetCount() const { return m_colors.size(); }

    private:
        std::vector<Rgb> m_colors;
    };

    class ColorRGBAImpl : public ImplObject<ColorRGBAImpl>
    {
    public:
        HRESULT Initialize(ColorRGBANode* node);
        void Shutdown();

        const std::vector<Rgba>& GetColors() const { return m_colors; }
        Rgba GetColor(size_t index) const;
        size_t GetCount() const { return m_colors.size(); }

    private:
        std::vector<Rgba> m_colors;
    };

    class TextureCoordinateImpl : public ImplObject<TextureCoordinateImpl>
    {
    public:
        HRESULT Initialize(TextureCoordinateNode* node);
        void Shutdown();

        const std::vector<Vec2>& GetPoints() const { return m_points; }
        Vec2 GetPoint(size_t index) const;
        size_t GetCount() const { return m_points.size(); }

    private:
        std::vector<Vec2> m_points;
    };

} // namespace HMREngine
