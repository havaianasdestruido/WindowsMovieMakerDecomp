/*
 * WLXFaceRecognition.h
 *
 * Public header for WLXFaceRecognition.dll.
 * Provides face detection and recognition for Windows Live Photo Gallery
 * and Movie Maker 2012. 4.6MB DLL containing the full ML pipeline.
 *
 * ML class hierarchy (LiveLabs namespace):
 *   CFaceDetector         -- Haar/DNN-based face region detection
 *   CFaceRecognizor       -- Face identity matching via feature vectors
 *   CNeuralNet            -- Neural network inference engine
 *   CClassifier           -- Generic classifier interface
 *   CLBPFeatureExtractor  -- Local Binary Pattern feature extraction
 *   PCAFeatureExtractor   -- Principal Component Analysis features
 *   CTextonFeatureExtractor -- Texton-based texture features
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXFACERECOGNITION_H
#define WLXFACERECOGNITION_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602

#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>

#ifdef WLXFACERECOGNITION_EXPORTS
    #define WLXFR_API __declspec(dllexport)
#else
    #define WLXFR_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace LiveLabs
{
    // Core ML classes
    class CFaceDetector;
    class CFaceRecognizor;
    class CNeuralNet;
    class CClassifier;

    // Feature extractors
    class CLBPFeatureExtractor;
    class PCAFeatureExtractor;
    class CTextonFeatureExtractor;

    // Supporting types
    struct FaceRegion;
    struct FaceFeature;
    struct RecognitionResult;
    struct DetectorConfig;
}

// ============================================================================
// Face region (rectangle + confidence)
// ============================================================================
struct WLXFaceRegion
{
    INT     x;
    INT     y;
    INT     cx;
    INT     cy;
    DOUBLE  dConfidence;     // 0.0 - 1.0
    UINT32  uFaceId;         // unique id assigned by detector

    WLXFaceRegion()
        : x(0), y(0), cx(0), cy(0)
        , dConfidence(0.0)
        , uFaceId(0)
    {}
};

// ============================================================================
// Recognition result
// ============================================================================
struct WLXRecognitionResult
{
    UINT32  uFaceId;
    WCHAR   wszPersonName[128];
    DOUBLE  dMatchScore;         // higher = better match
    UINT32  uPersonId;
    BOOL    bIsKnown;

    WLXRecognitionResult()
        : uFaceId(0)
        , dMatchScore(0.0)
        , uPersonId(0)
        , bIsKnown(FALSE)
    {
        ZeroMemory(wszPersonName, sizeof(wszPersonName));
    }
};

// ============================================================================
// Detector configuration
// ============================================================================
struct WLXDetectorConfig
{
    UINT32  uMinFaceSize;        // minimum face size in pixels
    UINT32  uMaxFaceSize;        // maximum face size in pixels
    DOUBLE  dScaleFactor;        // image pyramid scale factor
    UINT32  uMinNeighbors;       // Haar cascade min neighbors
    BOOL    bUseDNN;             // use deep neural network (if available)

    WLXDetectorConfig()
        : uMinFaceSize(20)
        , uMaxFaceSize(0)        // 0 = no limit
        , dScaleFactor(1.1)
        , uMinNeighbors(3)
        , bUseDNN(FALSE)
    {}
};

// ============================================================================
// Exported functions -- standard COM DLL entry points
// (DllCanUnloadNow, DllGetClassObject, DllRegisterServer, DllUnregisterServer
//  are defined in WLXFaceRecognition.cpp but not declared here to avoid
//  conflict with combaseapi.h declarations)
// ============================================================================

#endif // WLXFACERECOGNITION_H
