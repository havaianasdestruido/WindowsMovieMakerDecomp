# Cross-DLL GUID Reference Map

Generated: 2026-07-26
Scanned: 23 binaries (DLLs + EXEs) in `undecomp/Photo Gallery/` and `undecomp/Shared/`

---

## 1. GUID → Binary Cross-Reference

### Shared GUIDs (referenced by multiple binaries)

| GUID | Name | Defined In | Referenced By |
|------|------|-----------|---------------|
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim | WLXVideoTrim.dll | WLXPipeline.dll, WLXMediaPublishSubscribe.dll |
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER | ALL 23 binaries | (Authenticode cert) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER | ALL 23 binaries | (Authenticode cert) |
| `{AEE505D4-908A-4BFC-9A7E-31AF955C15BF}` | Imaging TypeLib | Imaging.dll | (all Imaging effect CLSIDs) |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | MediaPubSub TypeLib | WLXMediaPublishSubscribe.dll | (all MediaPubSub interfaces) |
| `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | PhotoAcq TypeLib | WLXPhotoAcq.dll | (all PhotoAcq CLSIDs) |
| `{77A16B7E-9DFE-410c-8863-264BF3E91103}` | GrinderScheduler TypeLib | WLXGrinderScheduler.dll | (GrinderScheduler) |
| `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | ImageTranscode TypeLib | WLXImageTranscode.dll | (ImageTranscode CLSIDs) |
| `{C6D340BB-0CEA-4923-8082-51036E472379}` | QuickTime TypeLib | WLXQuickTimeControlHost.exe | QuickTimePlayerHost, MovieThumbnail |

### Unique GUIDs (single binary)

| GUID | Binary |
|------|--------|
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | MovieMakerCore.dll only |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | MovieMakerCore.dll only |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | MovieMakerCore.dll only |
| `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` | WLXSlideshow.dll only |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | WLXFaceRecognition.dll only |
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | WLXFaceRecognition.dll only |
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | WLXFaceRecognition.dll only |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | WLXFaceRecognition.dll only |
| `{E30A45E6-1916-4659-95EE-035E62DB9AB0}` | WLXCodecHost.exe only |
| `{121244C5-B0AD-45fb-9D8E-B893B917D053}` | WLXGrinderScheduler.dll only |

---

## 2. Binary → GUID Summary

### WLXMediaPublishSubscribe.dll — 22 GUIDs

**CLSIDs (confirmed):**
| GUID | ProgID |
|------|--------|
| `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | Provider Manager Interface |
| `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | Live Provider Interface |
| `{197608E2-D42D-43a5-927E-1C67FC041431}` | Flickr Provider Interface |
| `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | Plugin Decorator Interface |
| `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | Plugin Manager Helper |
| `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | Online Media Plugin Manager |
| `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | Internet Cache Manager |
| `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | Publish Plugin Helper |
| `{DA69067E-3959-47ca-A58D-2300786168CD}` | Metadata Settings Controller |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | TypeLib IID |

**Other (from string scan):**
- `{8095E7A5-...}` (CLSID_VideoTrim reference)
- `{73647561-...}` (MEDIATYPE_Audio)
- Certificate GUIDs ×2
- Unknown/unassigned: `{0DDA997F...}`, `{1812A500...}`, `{1D31145D...}`, `{66557ED9...}`, `{7D2A6ECD...}`, `{8BE133F2...}`, `{8DB100C7...}`, `{D78480B9...}`

### Imaging.dll — 65+ GUIDs

All CLSIDs registered under TypeLib `{AEE505D4-908A-4BFC-9A7E-31AF955C15BF}`. Categories:
- **Standard Effects (39):** CacheEffect, CacheGridEffect, ChannelMixerEffect, ColorEngineEffect, ColorExposureEffect, ColorFillEffect, ColorManagementEffect, ColorSpaceConversionEffect, ColorTemperatureEffect, ContrastEffect, Convolve1DEffect, CropEffect, CreativeMaskEffect, CurveEffect, CurveSourceEffect, DenoiserAutoEffect, DenoiserRenderEffect, ExposureEffect, GaussianBlurEffect, GaussianBlurLargeRadiusEffect, HistogramEffect, HSEffect, HSFormulaEffect, ImageBufferSourceEffect, ImageEffect, InvertEffect, LevelsRGBEffect, LevelsYIQEffect, MipMapEffect, MultiCurveEffect, MultiplexerEffect, NoOpEffect, OutOfBoundsEffect, ProxyEffect, RedEyeEffect, RedEyeMaskEffect, RotateEffect, SaturationEffect, StraighteningEffect, TransformEffect, UnsharpMaskV2Effect, ViewEffect, ZoomHelperEffect
- **RAW/CaptureOne Pipeline (28):** PreprocessRawEffect, GreenGreenPostprocessEffect, GreenGreenCalibrationEffect, ChromaDenoiseEffect, CaptureOneChromaDenoiseEffect, CaptureOneRawColorEffect, CaptureOneEstimateGEffect, CaptureOneEstimateRBEffect, CaptureOneContrastEffect, BayerBinningEffect, CaptureOneBrightnessEffect, CaptureOneEdgeDetectEffect, CaptureOneGaussFilterEffect, CaptureOneSigmaFilterEffect, CaptureOneGDenoiseEffect, CaptureOneRBDenoiseEffect, CaptureOneExposureEffect, CaptureOneLevelsEffect, CaptureOneThresholdEffect, CaptureOneAntiColorAliasingEffect, CaptureOneBandingSuppressionEffect, CaptureOneGammaEffect, CaptureOneDemosaicEffect, CaptureOneSaturationEffect, CaptureOneHotPixelEffect, CaptureOneGGCalibrationEffect, RawContrastEffect, CaptureOneRawSceneEffect
- **Infrastructure (12):** CaptureOnePostProcessEffect, InverseRawSceneEffect, ProcessedSourceEffect, RawPipelineHistogramEffect, ProcessedPipelineEffect, CompositeSceneEffect, ImagingDeviceManager, D3DImageImpl, EffectBitmapRenderer, EffectCollection, EffectGraph, EffectGraphCopier, EffectHost, EffectInfo, EffectRegistrar, EffectRenderer, EnumEffect, ImageBufferCollection, ForegroundGraphicCollection, RectCollection, TileCollection, TileGenerator, DisplayView, EffectImpl, CompositeEffectImpl, GlobalResourceManager, LargeAllocMemoryManager, ImagingEngine, TraceManager, ApplyCurveEffect, BackgroundEffect, Base3dLutEffect, BaseCurveEffect, BlemishRemovalEffect, BlemishRemovalMaskEffect, BlendEffect

### WLXPhotoClassic.dll — 5 GUIDs

| GUID | Theme Name |
|------|-----------|
| `{773AFF18-2083-47C1-9EA9-A5DA346A0122}` | Classic |
| `{854E43AC-E1FD-46f2-8DD3-EE4C7A1844B6}` | Classic Transform |
| `{B9087BDF-F0F8-4454-A7D1-F6242E1654F8}` | Black and White |
| `{F91A0A3F-3E4E-4273-88CC-6664834ACA6F}` | Sepia |
| `{71ED30A7-499A-4F61-84F8-10CDEC657FE0}` | Basic |

### WLXPhotoVoyager.dll — 9 GUIDs

| GUID | Theme Name |
|------|-----------|
| `{97B9EC02-C47C-4996-A479-DD3DD31D572D}` | Voyager COM GUID |
| `{C84CFE1B-89DC-40e7-83BF-CB821255F9EC}` | Album |
| `{AEE6C573-A192-4af3-B62B-A4E6848533D3}` | Collage |
| `{653E52D8-D033-469a-8BB5-9C1A164416D5}` | Flip |
| `{B4E10BE6-A2CE-4bef-9D80-99995CB3C162}` | Frame |
| `{5515D2B5-6825-409e-B377-544708C9DD06}` | Glass |
| `{D5561752-E5A7-46e7-B768-D945E144CA78}` | Snapshots |
| `{CC4F1166-CE12-41f7-85E2-AE4744D9381B}` | Travel |
| `{E48325CB-1EFC-425e-9CD9-47EF51BECD55}` | Voyager Transform |

### WLXPhotoAcq.dll — 7 GUIDs

| GUID | Class Name |
|------|-----------|
| `{4E13844B-6012-40cb-8E3C-0F7EB9AEF6DD}` | LivePhotoAcquire |
| `{94B1CBEA-F33F-4508-8AAA-55FD8113EFCB}` | LivePhotoAcquireOptionsDialog |
| `{0D5A7D0E-9A06-4e17-85D9-A0B24036371D}` | LivePhotoPickerDialog |
| `{E84D0D46-3D57-4039-9EFE-310AF1CAF92A}` | LivePhotoAcqDeviceSelectionDlg |
| `{4D8A134F-3D0A-4375-8B1A-78CD171C9318}` | LivePhotoAcquisitionWizard |
| `{3BD0ACD1-71CA-4475-92CC-E0AA0AAF843F}` | CLSIDForCancel |
| TypeLib: `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | — |

### WLXGrinderScheduler.dll — 2 GUIDs

| GUID | Class Name |
|------|-----------|
| `{121244C5-B0AD-45fb-9D8E-B893B917D053}` | Grinder Scheduler |
| TypeLib: `{77A16B7E-9DFE-410c-8863-264BF3E91103}` | — |

### WLXImageTranscode.dll — 8 GUIDs

| GUID | Class Name |
|------|-----------|
| `{20575516-78AF-4404-B3C7-51D05F9945B5}` | ImageTranscode |
| `{E253845C-A1A2-4ae4-B784-5E8140CD5B2C}` | ImageLoader |
| `{3FBB103C-F1B9-47dc-9EB3-A0C07F5F6AFA}` | PSFactoryBuffer |
| `{CB38B8DF-0D64-42b9-802A-28DCB678BFEB}` | IWLXImageLoader (IID) |
| `{B8A2E14E-290D-4122-B092-1A7D86198CCE}` | WLXOutofProc |
| TypeLib: `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | — |

### WLXCodecHost.exe — 1 GUID

| GUID | Class Name |
|------|-----------|
| `{E30A45E6-1916-4659-95EE-035E62DB9AB0}` | Codec Host |

### WLXQuickTimeControlHost.exe — 3 GUIDs

| GUID | Class Name |
|------|-----------|
| `{B9AD19CB-FA75-4B29-B4A4-86C7E9616390}` | QuickTimePlayerHost |
| `{AE3A66BB-85FE-49B8-BF7B-4DB4E0005091}` | QuickTimeMovieThumbnail |
| TypeLib: `{C6D340BB-0CEA-4923-8082-51036E472379}` | — |

### WLXVideoAcquireWizard.exe — 5 GUIDs

| GUID | Class Name |
|------|-----------|
| `{5ab7566d-f75b-4a53-9615-115b6cb1d59b}` | AppID |
| `{5abe6468-4a2a-403c-892d-06e1fc31097f}` | MSLive Capture Wizard |
| `{5ab23fca-6040-4012-8fea-8da67f5806a7}` | MSLive Auto Capture |
| TypeLib: `{5ab7792c-0f76-4003-aa47-5f075165d4de}` | — |

### WLXVideoCameraAutoPlayManager.exe — 2 GUIDs

| GUID | Class Name |
|------|-----------|
| `{9b5c97f6-b3a5-4a6d-8b03-993ec7291a22}` | WLXHWEventHandler |
| TypeLib: `{9b5c8343-bdee-475d-9d3b-3715c6b8972e}` | — |

### WLXDSPA.dll — 11 GUIDs

| GUID | Class Name |
|------|-----------|
| `{BC8CA1B3-B013-4866-9621-825957DF23F3}` | CWLXTocGeneratorDmo |
| `{09BC59C2-70DD-45f9-A5B7-DE9F2A5CA34B}` | CWLXThumbnailGeneratorDmo |
| `{15CD2459-C14B-457b-B57B-3DBA111B9D09}` | CClusterDetectorEx |
| `{1D8D19C8-0A33-45a4-9B3E-255B85C363A8}` | CTocEntry |
| `{15A4E6E5-A9E5-49cb-AFFC-E822F082D427}` | CTocEntryList |
| `{C9FF4813-CB5F-4ac6-B003-4D79AE2F43E9}` | CToc |
| `{CE1D8A09-77EA-4eaa-9619-832A9E5DB447}` | CTocCollection |
| `{75704D6C-09BA-4d19-AFEA-5F21FC08B3DB}` | CTocParser |
| `{7F2CE947-4E80-446d-9AE4-17DD9D82A353}` | CFileIo |
| `{BF620143-7420-460a-9EEE-178B78D4939D}` | CAsfTocParser |
| `{9FAE79C9-BA02-43d9-9382-C7BEF740A596}` | CAviTocParser |

### WLAVRes.dll — 8 GUIDs

| GUID | Threading |
|------|-----------|
| `{48e2ed0f-98c2-4a37-bed5-166312ddd83f}` | Both |
| `{f792beee-aeaf-4ebb-ab14-8bc5c8c695a8}` | Both |
| `{0344ec28-5339-4124-a186-2e8eef168785}` | Both |
| `{743a6e3b-a5df-43ed-b615-4256add790b8}` | Both |
| `{eb4d075a-65c0-476b-956c-c605eade03f7}` | Both |
| `{14d7a407-396b-44b3-be85-5199a0f0f80a}` | Both |
| `{354FDED2-09E6-41df-9839-EFD82DCD2533}` | Both |
| `{B5754F1A-D387-49D0-B32D-B5A7633FF3D4}` | Both |

---

## 3. TypeLib Summary

| TypeLib GUID | DLL | # CLSIDs |
|-------------|-----|----------|
| `{AEE505D4-908A-4BFC-9A7E-31AF955C15BF}` | Imaging.dll | ~65 |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | WLXMediaPublishSubscribe.dll | 8 |
| `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | WLXPhotoAcq.dll | 5 |
| `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | WLXImageTranscode.dll | 3 |
| `{C6D340BB-0CEA-4923-8082-51036E472379}` | WLXQuickTimeControlHost.exe | 2 |
| `{5ab7792c-0f76-4003-aa47-5f075165d4de}` | WLXVideoAcquireWizard.exe | 2 |
| `{9b5c8343-bdee-475d-9d3b-3715c6b8972e}` | WLXVideoCameraAutoPlayManager.exe | 1 |
| `{77A16B7E-9DFE-410c-8863-264BF3E91103}` | WLXGrinderScheduler.dll | 1 |
| `{60E1FA84-4F2F-417C-AEE4-7681A960D09E}` | AlbumDownloadProtocolHandler.dll | 1 |

---

## 4. Known GUIDs NOT Found via String Scanning

These GUIDs exist in binaries as 16-byte binary structs or in .rgs resources, not as searchable strings:

- **CLSID_CinematicFullScreen** `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` — found via .rgs in WLXPhotoCinematic.dll
- **CLSID_CinematicTransform** `{5409AB48-D8D3-40e6-A1EB-23489DC422DE}` — found via .rgs in WLXPhotoCinematic.dll
- **Transition DLL GUIDs** `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}`, `{7371ADEE-C195-427F-B0EC-3CCC13725665}` — embedded in XML resources of WLXPhotoCinematic.dll
- **All PDB GUIDs** — stored as 16-byte binary structs in debug directories, not as strings
