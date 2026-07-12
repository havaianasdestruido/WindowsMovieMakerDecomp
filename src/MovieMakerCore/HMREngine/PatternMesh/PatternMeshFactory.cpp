// PatternMeshFactory.cpp - PatternMesh factory implementation

#include "PatternMeshFactory.h"

namespace HMREngine
{
namespace DX
{

std::unique_ptr<PatternMesh> PatternMeshFactory::Create(TransitionType type)
{
    switch (type)
    {
    case TransitionType::Checkerboard:        return std::make_unique<CheckerboardPatternMesh>();
    case TransitionType::BowTie:              return std::make_unique<BowTiePatternMesh>();
    case TransitionType::Circles:             return std::make_unique<CirclesPatternMesh>();
    case TransitionType::Circle:              return std::make_unique<CirclePatternMesh>();
    case TransitionType::DiagonalCross:       return std::make_unique<DiagonalCrossPatternMesh>();
    case TransitionType::DiagonalBox:         return std::make_unique<DiagonalBoxPatternMesh>();
    case TransitionType::FanAndSweep:         return std::make_unique<FanAndSweepPatternMesh>();
    case TransitionType::FanAndSweepBase:     return std::make_unique<FanAndSweepBasePatternMesh>();
    case TransitionType::FanIn:               return std::make_unique<FanInPatternMesh>();
    case TransitionType::Eye:                 return std::make_unique<EyePatternMesh>();
    case TransitionType::FanUp:               return std::make_unique<FanUpPatternMesh>();
    case TransitionType::FanOut:              return std::make_unique<FanOutPatternMesh>();
    case TransitionType::Heart:               return std::make_unique<HeartPatternMesh>();
    case TransitionType::FillV:               return std::make_unique<FillVPatternMesh>();
    case TransitionType::Keyhole:             return std::make_unique<KeyholePatternMesh>();
    case TransitionType::Iris:                return std::make_unique<IrisPatternMesh>();
    case TransitionType::Rectangles:          return std::make_unique<RectanglesPatternMesh>();
    case TransitionType::Rectangle:           return std::make_unique<RectanglePatternMesh>();
    case TransitionType::Split:               return std::make_unique<SplitPatternMesh>();
    case TransitionType::Reveal:              return std::make_unique<RevealPatternMesh>();
    case TransitionType::Stars:               return std::make_unique<StarsPatternMesh>();
    case TransitionType::Star:                return std::make_unique<StarPatternMesh>();
    case TransitionType::SweepIn:             return std::make_unique<SweepInPatternMesh>();
    case TransitionType::SweepOut:            return std::make_unique<SweepOutPatternMesh>();
    case TransitionType::Wheel:               return std::make_unique<WheelPatternMesh>();
    case TransitionType::SweepUp:             return std::make_unique<SweepUpPatternMesh>();
    case TransitionType::Zigzag:              return std::make_unique<ZigzagPatternMesh>();
    default:                                  return nullptr;
    }
}

const char* PatternMeshFactory::GetTransitionName(TransitionType type)
{
    auto mesh = Create(type);
    return mesh ? mesh->GetTransitionName() : "Unknown";
}

UINT PatternMeshFactory::GetTransitionCount()
{
    return 27;
}

TransitionType PatternMeshFactory::GetTransitionByIndex(UINT index)
{
    if (index >= GetTransitionCount()) return TransitionType::Checkerboard;
    return static_cast<TransitionType>(index);
}

} // namespace DX
} // namespace HMREngine
