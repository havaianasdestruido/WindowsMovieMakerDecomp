#pragma once
// PatternMeshFactory.h - Factory that maps transition type enum to PatternMesh subclass

#include "PatternMesh.h"
#include <memory>

namespace HMREngine
{
    namespace DX
    {
        class PatternMeshFactory
        {
        public:
            static std::unique_ptr<PatternMesh> Create(TransitionType type);
            static const char* GetTransitionName(TransitionType type);
            static UINT GetTransitionCount();
            static TransitionType GetTransitionByIndex(UINT index);
        };

    } // namespace DX
} // namespace HMREngine
