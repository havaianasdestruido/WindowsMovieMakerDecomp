#pragma once
// PatternMesh.h - Wipe transition pattern meshes (27 pattern types)

#include "../DXResources/DXResources.h"
#include "../DXResources/MeshResourceDX.h"
#include <vector>
#include <memory>

namespace HMREngine
{
    namespace DX
    {
        // --- Transition type enum ---
        enum class TransitionType
        {
            Checkerboard,
            BowTie,
            Circles,
            Circle,
            DiagonalCross,
            DiagonalBox,
            FanAndSweep,
            FanAndSweepBase,
            FanIn,
            Eye,
            FanUp,
            FanOut,
            Heart,
            FillV,
            Keyhole,
            Iris,
            Rectangles,
            Rectangle,
            Split,
            Reveal,
            Stars,
            Star,
            SweepIn,
            SweepOut,
            Wheel,
            SweepUp,
            Zigzag,
        };

        // --- PatternMesh: Base class for all wipe transition patterns ---
        class PatternMesh
        {
        public:
            PatternMesh();
            virtual ~PatternMesh();

            virtual HRESULT Generate(MeshResourceDX* mesh, UINT width, UINT height);
            virtual HRESULT Generate(ID3D11Device* dev, UINT width, UINT height);
            virtual void Release();

            virtual TransitionType GetTransitionType() const = 0;
            virtual const char* GetTransitionName() const = 0;

            void SetProgress(float progress) { m_progress = Clamp(progress, 0.0f, 1.0f); }
            float GetProgress() const { return m_progress; }

            void SetSoftEdge(float soft) { m_softEdge = Clamp(soft, 0.0f, 1.0f); }
            float GetSoftEdge() const { return m_softEdge; }

            ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer; }
            ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer; }
            UINT GetVertexCount() const { return m_vertexCount; }
            UINT GetIndexCount() const { return m_indexCount; }

        protected:
            CComPtr<ID3D11Buffer> m_vertexBuffer;
            CComPtr<ID3D11Buffer> m_indexBuffer;
            UINT m_vertexCount = 0;
            UINT m_indexCount = 0;
            float m_progress = 0.0f;
            float m_softEdge = 0.1f;

            struct PatternVertex
            {
                Vec3 position;
                Vec2 texCoord;
                float mask;
                float feather;
                float padding;
            };

            virtual void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) = 0;
            virtual void BuildIndices(std::vector<WORD>& indices) = 0;

            HRESULT CreateBuffers(ID3D11Device* dev, const std::vector<PatternVertex>& vertices,
                const std::vector<WORD>& indices);
            float ApplySoftEdge(float mask) const;
            float ComputeRadialMask(float x, float y, float cx, float cy, float radius) const;
            float ComputeAngularMask(float x, float y, float cx, float cy, float startAngle, float sweep) const;
        };

        // --- Concrete pattern meshes ---

        class CheckerboardPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Checkerboard; }
            const char* GetTransitionName() const override { return "Checkerboard"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
            UINT m_gridSize = 8;
        };

        class BowTiePatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::BowTie; }
            const char* GetTransitionName() const override { return "BowTie"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class CirclesPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Circles; }
            const char* GetTransitionName() const override { return "Circles"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
            UINT m_numCircles = 12;
        };

        class CirclePatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Circle; }
            const char* GetTransitionName() const override { return "Circle"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class DiagonalCrossPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::DiagonalCross; }
            const char* GetTransitionName() const override { return "DiagonalCross"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class DiagonalBoxPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::DiagonalBox; }
            const char* GetTransitionName() const override { return "DiagonalBox"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class FanAndSweepPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::FanAndSweep; }
            const char* GetTransitionName() const override { return "FanAndSweep"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class FanAndSweepBasePatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::FanAndSweepBase; }
            const char* GetTransitionName() const override { return "FanAndSweepBase"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class FanInPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::FanIn; }
            const char* GetTransitionName() const override { return "FanIn"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class EyePatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Eye; }
            const char* GetTransitionName() const override { return "Eye"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class FanUpPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::FanUp; }
            const char* GetTransitionName() const override { return "FanUp"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class FanOutPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::FanOut; }
            const char* GetTransitionName() const override { return "FanOut"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class HeartPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Heart; }
            const char* GetTransitionName() const override { return "Heart"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class FillVPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::FillV; }
            const char* GetTransitionName() const override { return "FillV"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class KeyholePatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Keyhole; }
            const char* GetTransitionName() const override { return "Keyhole"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class IrisPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Iris; }
            const char* GetTransitionName() const override { return "Iris"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class RectanglesPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Rectangles; }
            const char* GetTransitionName() const override { return "Rectangles"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
            UINT m_cols = 6;
            UINT m_rows = 4;
        };

        class RectanglePatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Rectangle; }
            const char* GetTransitionName() const override { return "Rectangle"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class SplitPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Split; }
            const char* GetTransitionName() const override { return "Split"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class RevealPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Reveal; }
            const char* GetTransitionName() const override { return "Reveal"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class StarsPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Stars; }
            const char* GetTransitionName() const override { return "Stars"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
            UINT m_numStars = 16;
        };

        class StarPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Star; }
            const char* GetTransitionName() const override { return "Star"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class SweepInPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::SweepIn; }
            const char* GetTransitionName() const override { return "SweepIn"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class SweepOutPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::SweepOut; }
            const char* GetTransitionName() const override { return "SweepOut"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class WheelPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Wheel; }
            const char* GetTransitionName() const override { return "Wheel"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
            UINT m_spokes = 12;
        };

        class SweepUpPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::SweepUp; }
            const char* GetTransitionName() const override { return "SweepUp"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
        };

        class ZigzagPatternMesh : public PatternMesh
        {
        public:
            TransitionType GetTransitionType() const override { return TransitionType::Zigzag; }
            const char* GetTransitionName() const override { return "Zigzag"; }
        protected:
            void BuildVertices(std::vector<PatternVertex>& vertices, UINT width, UINT height) override;
            void BuildIndices(std::vector<WORD>& indices) override;
            UINT m_zigzagRows = 8;
        };

    } // namespace DX
} // namespace HMREngine
