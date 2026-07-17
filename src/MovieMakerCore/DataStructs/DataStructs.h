/*
 * DataStructs.h
 *
 * Root header for the Base::DataStructs namespace within MovieMakerCore.dll.
 * Provides fundamental data structure classes used throughout the engine:
 *   - IntSet: sorted integer set for extent ID tracking
 *   - StringSet: string set for media item tracking
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#ifndef STRICT
#define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602
#define _WIN32_IE     0x0800

#include <windows.h>
#include <objbase.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>

#include "WLXPhotoBase.h"
#include "../MovieMakerCore.h"

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4505) // unreferenced local function has been removed

// ============================================================================
// Base::DataStructs namespace
// ============================================================================
// Fundamental data structures used across MovieMakerCore subsystems.
// These are low-level utility types that support the higher-level
// StoryboardManager and HMRAVSource namespaces.
//
namespace Base
{
namespace DataStructs
{

// Forward declarations
class IntSet;
class StringSet;

} // namespace DataStructs
} // namespace Base

// ============================================================================
// Include sub-headers
// ============================================================================
#include "IntSet.h"
#include "StringSet.h"

#pragma warning(pop)

#endif // DATASTRUCTS_H
