---
sidebar_position: 5
title: Per-Module C++ Tests & CTest
description: The classic tests/<Module>/ C++ executables and the CTest sanity gate.
---

# Per-Module C++ Tests & CTest

## The `tests/<Module>/` directories

Alongside the three harness submodules sit **classic C++ test executables**, one
directory per module:

```text
tests/MovieMakerCore/test_core.cpp
tests/WLMFDS/        tests/WLMFReadWrite/   tests/WLXCodecHost/
tests/WLXCodecHostPS/tests/WLXDSPA/        tests/WLXFaceRecognition/
tests/WLXGrinderScheduler/tests/WLXImageTranscode/tests/WLXMP4Parser/
tests/WLXMediaPublishSubscribe/ tests/WLXMovieLibrary/ tests/WLXPhotoAcq/
tests/WLXPhotoBase/ tests/WLXPhotoCinematic/tests/WLXPhotoClassic/
tests/WLXPhotoGalleryRepair/tests/WLXPhotoLibraryDatabase/tests/WLXPhotoLibraryMain/
tests/WLXPhotoSqm/  tests/WLXPhotoViewer/  tests/WLXPhotoVoyager/
tests/WLXPipeline/  tests/WLXPipetran/     tests/WLXQuickTime/
tests/WLXSendMail/  tests/WLXSlideshow/    tests/WLXTranscode/
tests/WLXVAFilt/    tests/WLXVideoAcquireWizard/tests/WLXVideoCameraAutoPlayManager/
tests/WLXVideoTrim/ tests/AlbumDownloadProtocolHandler/tests/DmxBici/
tests/MovieMakerPreviewClient/ tests/NPWLPG/ tests/PhotoViewerShim/
tests/SharedMFDlls/ tests/OtherDlls/       tests/wlsoxe/ tests/wlxclip/
```

These are **legacy-style console tests** (`test_core.cpp` pattern): link against the
module, call the surface, print PASS/FAIL. They predate the contract suite and remain
useful for debugging a single module with a debugger attached.

Some directories cover binaries whose source is not (yet) reconstructed (e.g.
`WLXDSPA`, `WLXPhotoAcq`, `NPWLPG`) — they hold analysis-facing tests and documentation
for those surfaces.

## CTest

The root `CMakeLists.txt` wires two sanity tests via `include(CTest)`:

```cmake
add_test(NAME sanity_MovieMaker_exe_exists ...)
add_test(NAME sanity_MovieMakerCore_dll_exists ...)
```

(Implemented as `compare_files` self-comparisons of the target files — they assert the
outputs exist at the expected paths.)

Run:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

Expected: **2/2 pass**. CTest is a smoke gate, not the behavioral oracle — the
[contract suite](contract-suite.md) holds that role.
