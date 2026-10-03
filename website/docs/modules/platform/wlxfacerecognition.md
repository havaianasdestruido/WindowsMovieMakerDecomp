---
sidebar_position: 3
title: WLXFaceRecognition.dll
description: Face detection and recognition with bounded analysis buffers — and corrected CLSID labels.
---

# WLXFaceRecognition.dll

| | |
|---|---|
| **Source** | `src/WLXFaceRecognition/` |
| **CMake target** | `WLXFaceRecognition` |
| **Type** | Win32 COM DLL |
| **Family** | [Platform services targets](../platform) |

## At a glance

**Face detection/recognition** over photos and video frames — powering face tagging and
auto-suggested people tags. The pipeline recognizes faces, groups them into
`FaceRegion`s / `FaceRegionSet`s, and matches against known identities.

## Public surface

The standard COM quartet (`WLXFaceRecognition.def`).

## Corrected COM GUIDs (worth knowing)

A 2026-07-26 cross-check of binary strings against `.rgs` registry scripts **corrected
two CLSID labels** that had been swapped in early analysis (full detail in
`analysis/COMGuids/guids.md`):

| GUID | Old (wrong) label | Corrected label |
|---|---|---|
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | ~~CLSID_FaceRegion / FaceRegionSet~~ | **CLSID_FaceRecognitionPipeline** |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | ~~CLSID_FaceRecognitionPipeline~~ | **TypeLib IID** |

## Bounded analysis (security)

Recognition runs on **downscaled buffers with a 640 px max dimension** and bounded
sample counts — untrusted images cannot blow up memory or time
(see [Security](../../reference/security.md#untrusted-input-bounds)). This module is one
of the four precedent cases for the project's bounded-parser policy.

## Testing

- `tests/WLXFaceRecognition/` — per-module C++ tests.
- Contract coverage for the pipeline CLSID + factory in `tests/mmr-python`.
- Exercise app: `apps/face-tagger` (submodule).

## Analysis artifacts

`analysis/WLXFaceRecognition/`, `analysis/COMGuids/`.
