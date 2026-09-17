# WLXFaceRecognition

Face detection/recognition DLL. Haar-inspired cascade + LBP/PCA/texton
feature extractors, blob filtering, and a deterministic RNG for codebook
construction (reproducible output).

## Hardening

- Analysis resolution capped at **640px max dimension** (config-scaled
  min-face-size); runtime stays bounded for 1080p+ video.
- Feature sampler caps at **4096** pixels/samples; texton codebook bounded
  by sample count.
- LBP/PCA/texton extractors index images via `static_cast<size_t>(y) * w`
  (64-bit index math, no 32-bit overflow).

## Contract Coverage

See `analysis/WLXFaceRecognition/` for the behavioral harness; the public
detection/recognition pipeline is exercised by `run_tests.py` workflows.