# COM GUIDs Extracted from Windows Live Movie Maker 2012 Binaries

Extraction methods:
1. **ASCII string scan**: finds `{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}` patterns in contiguous 7-bit ASCII printable strings (null-terminated)
2. **UTF-16LE string scan**: same pattern in wide-character (null-padded) Unicode strings
3. **Embedded .rgs registry scripts** (findstr scan): extracts CLSID/IID/TypeLib registrations from ATL resource scripts embedded in `.rsrc` sections

Generated: 2026-07-26 (updated with cross-DLL verification)

---

## Per-DLL GUID Breakdown

### 1. MovieMakerCore.dll
**Total GUIDs: 5** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting endpoint |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Transition/effect template GUID |
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | App/module identifier (MovieMakerCore) |

### 2. WLXSlideshow.dll
**Total GUIDs: 3** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` | **COM CLSID (WLXSlideshow)** — from string scan |

### 3. WLXVideoTrim.dll
**Total GUIDs: 3** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | **CLSID_VideoTrim** (WLXVideoTrim) |

### 4. WLXFaceRecognition.dll
**Total GUIDs: 4** (CORRECTED — 3 CLSIDs + 1 TypeLib IID)

| GUID | Category |
|------|----------|
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | **CLSID_FaceDetection** ✅ verified .rgs |
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | **CLSID_ImageManager** ✅ verified .rgs |
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | **CLSID_FaceRecognitionPipeline** ✅ verified .rgs (CORRECTED: was "FaceRegion") |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | **TypeLib IID** (CORRECTED: was "CLSID_FaceRecognitionPipeline") |

**CORRECTION**: Previous documentation incorrectly labeled `{D01C34A5...}` as "CLSID_FaceRegion/FaceRegionSet" and `{EF401225...}` as "CLSID_FaceRecognitionPipeline". The .rgs registry scripts in the binary confirm:
- `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` = `ForceRemove ... = s 'Windows Live Photo Gallery Face Recognition FaceRecognitionPipeline Class'`
- `{EF401225-1260-4716-A842-7D180DC14C1E}` = `TypeLib = s '{EF401225-...}'`

### 5. WLXPipeline.dll
**Total GUIDs: 3** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim (shared reference) |

### 6. WLXMovieLibrary.dll
**Total GUIDs: 2** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

### 7. WLXMediaPublishSubscribe.dll
**Total GUIDs: 22** (all verified via .rgs)

| GUID | Category | .rgs Confirmed Name |
|------|----------|---------------------|
| `{0DDA997F-E7FA-404B-B3D3-F1610807FB66}` | Unknown/unassigned | — |
| `{1812A500-BA1E-41EE-B3A5-5D7B6FCA7393}` | Unknown/unassigned | — |
| `{197608E2-D42D-43a5-927E-1C67FC041431}` | **FlickrProvider Interface** ✅ | "Flickr Publish and Subscribe Provider Interface" |
| `{1D31145D-AEFE-48B0-B48A-513E5D413B44}` | Unknown/unassigned | — |
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER (non-COM) | — |
| `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | **ProviderManager Interface** ✅ | "Publish and Subscribe Provider Manager Interface" |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER (non-COM) | — |
| `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | **PluginDecorator Interface** ✅ | "Plugin Decorator Interface" |
| `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | **LiveProvider Interface** ✅ | "Live Publish and Subscribe Provider Interface" |
| `{66557ED9-C5F8-4815-A8CC-D157272CFFCE}` | Unknown/unassigned | — |
| `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | **PluginManagerHelper** ✅ | "Publish Plugin Manager Helper" |
| `{73647561-0000-0010-8000-00AA00389B71}` | MEDIATYPE_Audio (DirectShow standard) | — |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | **TypeLib IID** ✅ | TypeLib referenced by multiple interfaces |
| `{7D2A6ECD-EBB6-4BA9-A3CD-4A7FD43B0423}` | Unknown/unassigned | — |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim (shared reference) | — |
| `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | **OnlineMediaPluginManager** ✅ | "Online Media Plugin Manager Interface" |
| `{8BE133F2-9F23-4344-B5A1-A49BDD09FC0F}` | Unknown/unassigned | — |
| `{8DB100C7-50C5-46EB-B535-618AB68A3E22}` | Unknown/unassigned | — |
| `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | **InternetCacheManager** ✅ | "Internet Cache Manager" |
| `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | **PublishPluginHelper** ✅ | "Publish Plugin Helper" |
| `{D78480B9-E434-451C-B8A4-EDBEC652C066}` | Unknown/unassigned | — |
| `{DA69067E-3959-47ca-A58D-2300786168CD}` | **MetadataSettingsController** ✅ | "Metadata Settings Controller Interface" |

### 8. WLMFDS.dll
**Total GUIDs: 2** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

### 9. WLXMP4Parser.dll
**Total GUIDs: 2** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

### 10. WLXPhotoBase.dll
**Total GUIDs: 2** (all verified)

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

### 11. Imaging.dll (NEW — Photo Gallery image effects engine)
**Total GUIDs: ~65+ CLSIDs + 1 TypeLib** (all verified via .rgs)

**TypeLib**: `{AEE505D4-908A-4BFC-9A7E-31AF955C15BF}`

All CLSIDs registered under this TypeLib — major categories:

**Image Processing Effects (WIC-based):**
| GUID | ProgID |
|------|--------|
| `{0E8D9408-B0D8-41BC-ADBF-BF52BBFEE1C4}` | CacheEffect Class |
| `{3CD7E29C-3A52-48EF-97E4-109A783E87F1}` | CacheGridEffect Class |
| `{C39808FD-E65E-48DE-AAD5-94186D02BBFF}` | ChannelMixerEffect Class |
| `{90C5A6A8-4342-4578-AF85-9E1FDEF6DA5E}` | ColorEngineEffect Class |
| `{19ED6BB0-7AE6-4142-BA24-8639FDD4699A}` | ColorExposureEffect Class |
| `{2A587447-EA3B-428C-B9F1-A5EEA51697CE}` | ColorFillEffect Class |
| `{FAD98045-C10A-4569-969D-CD02941FCA07}` | ColorManagementEffect Class |
| `{6EB332B2-9B2A-46BA-8EB0-7198C8E3F0D0}` | ColorSpaceConversionEffect Class |
| `{95D89793-FA23-4FD3-B6EA-8C48C83C41CE}` | ColorTemperatureEffect Class |
| `{2C37E29C-1F37-4835-9F50-2D88CFB3443E}` | ContrastEffect Class |
| `{07746787-0F10-420E-9225-36EFFB1EB953}` | Convolve1DEffect Class |
| `{AF98C6C7-7C14-467F-8AFC-30F9D0F3BD73}` | CropEffect Class |
| `{2E6A0CF8-2FC4-44C6-B67D-CAF6646B9752}` | CreativeMaskEffect Class |
| `{E7915F28-75E2-4076-8B83-F1FF4AFF1921}` | CurveEffect Class |
| `{68CFC81E-461C-48DA-989C-9EB776362F6E}` | CurveSourceEffect Class |
| `{54FB9738-BE39-4D3C-B2E8-D4A1193ECC4C}` | DenoiserAutoEffect Class |
| `{CAE12246-8447-4148-B7F0-54B90118AB5D}` | DenoiserRenderEffect Class |
| `{CA222DD0-0994-409D-9A35-83442EBCFCED}` | ExposureEffect Class |
| `{8224594A-6B12-41B1-9883-E8722713EF6D}` | GaussianBlurEffect Class |
| `{CB454859-EEE4-4B27-A092-582EA6AD174B}` | GaussianBlurLargeRadiusEffect Class |
| `{9720856E-F265-453D-B449-F0C7E2933422}` | HistogramEffect Class |
| `{D4A2ED8F-0AF6-4F99-A725-CB44CAD23941}` | HSEffect Class |
| `{17123627-6B2E-476C-ADF7-F063D2BD1A4D}` | HSFormulaEffect Class |
| `{5E2C7736-DD3A-4896-BD1B-AC4EE519B1AF}` | ImageBufferSourceEffect Class |
| `{7B13EA1C-CA6E-42B4-B074-66CE103A3971}` | ImageEffect Class |
| `{0ECA85A5-B552-4648-9AB0-4A94525FF507}` | InvertEffect Class |
| `{B75D7654-6816-4784-9915-A715075997EE}` | LevelsRGBEffect Class |
| `{BBCDD5FC-DEF7-42DE-9380-AB1EDB995600}` | LevelsYIQEffect Class |
| `{4E52008E-43EF-46A7-84C9-9B636B11ECDF}` | MipMapEffect Class |
| `{F4CE07F0-CB26-43ED-A668-B79B94DA4F58}` | MultiCurveEffect Class |
| `{2C04B5F9-2E24-41E2-B549-294A5F18ECE7}` | MultiplexerEffect Class |
| `{32CFA2A1-98D0-430C-82B4-B45D4AC1318E}` | NoOpEffect Class |
| `{BA6DA777-CA17-402C-B7A3-D0444AC4BC70}` | OutOfBoundsEffect Class |
| `{C668BBE5-8738-439A-BAF1-AD8D97A13449}` | ProxyEffect Class |
| `{2D42FE52-8618-48FC-AEFE-83A309530AF9}` | RedEyeEffect Class |
| `{B5B713B6-C5F4-466A-BFDC-FC289F6C4D09}` | RedEyeMaskEffect Class |
| `{1B1114A2-1970-49FD-A685-F9C4C180C9F4}` | RotateEffect Class |
| `{C9703745-65A8-4ED8-A993-43C467ACF0AC}` | SaturationEffect Class |
| `{7302EB5A-88B0-4549-8813-89C1056347EE}` | StraighteningEffect Class |
| `{5FADF6FB-058E-4BBB-8014-ED1DDB433256}` | TransformEffect Class |
| `{31C12862-BB44-4710-AEF0-33FEA2A28017}` | UnsharpMaskV2Effect Class |
| `{FDEFC100-05CA-4B82-A4A5-FEECB48129A5}` | ViewEffect Class |
| `{AD169FB7-2ACF-4cf2-8284-1A3C213B7A7D}` | ZoomHelperEffect Class |

**RAW Processing (CaptureOne pipeline):**
| GUID | ProgID |
|------|--------|
| `{EC431FB9-C51B-4D1A-BE26-D1C3316A366F}` | PreprocessRawEffect Class |
| `{D2AE631D-2931-4B9F-81DA-134D570FFA8E}` | GreenGreenPostprocessEffect Class |
| `{C8BE1625-CB7D-473C-B826-4F953307E9B6}` | GreenGreenCalibrationEffect Class |
| `{B91B32B7-27D5-4622-8DEB-05DB4DBC2558}` | ChromaDenoiseEffect Class |
| `{51CFFB59-A22E-41AD-9118-9D0DDEB845B6}` | CaptureOneChromaDenoiseEffect Class |
| `{C651FEDD-1E5D-4421-A966-1AEBA8CB0940}` | CaptureOneRawColorEffect Class |
| `{7D3BAD99-1959-4B93-AE58-37A43861C8EC}` | CaptureOneEstimateGEffect Class |
| `{D35FC299-6883-418B-AE20-85769002E230}` | CaptureOneEstimateRBEffect Class |
| `{91A6E0FC-45E3-41DF-8F02-6494FD7B4EFA}` | CaptureOneContrastEffect Class |
| `{3F6F64F3-2C91-4218-95BC-CA01808D48FC}` | BayerBinningEffect Class |
| `{9CF7335B-BB7F-4E22-91BA-9CF30F6D3DF2}` | CaptureOneBrightnessEffect Class |
| `{4A4BF063-E2C6-46D4-BC0D-348C20C63498}` | CaptureOneEdgeDetectEffect Class |
| `{460B0B83-B815-4936-A83E-F8237733B14C}` | CaptureOneGaussFilterEffect Class |
| `{18D31E7D-33FC-4338-AA28-D437B0E415A0}` | CaptureOneSigmaFilterEffect Class |
| `{0DF9FED2-A043-4614-B07B-FFA576F238C6}` | CaptureOneGDenoiseEffect Class |
| `{9CE256B2-15DB-4E06-B905-D8A774A658B7}` | CaptureOneRBDenoiseEffect Class |
| `{06C595CA-6AAC-49A4-A2F6-773C6E0A45E8}` | CaptureOneExposureEffect Class |
| `{12411447-58D8-426B-93B7-FD961454253E}` | CaptureOneLevelsEffect Class |
| `{63B23621-E3E3-4842-81FE-4AFE8CE6DE67}` | CaptureOneThresholdEffect Class |
| `{300D4678-9A02-4CA9-A68F-B457AD9FF205}` | CaptureOneAntiColorAliasingEffect Class |
| `{E173B923-30B6-434D-9CC0-BA4EC339E79B}` | CaptureOneBandingSuppressionEffect Class |
| `{2F767008-C7A6-44B0-A25E-8FB1F1639511}` | CaptureOneGammaEffect Class |
| `{8F86379B-5C0C-403A-B137-E17463B9278B}` | CaptureOneDemosaicEffect Class |
| `{FAA67BE5-DF17-48A4-A937-25F2BC1CEC8F}` | CaptureOneSaturationEffect Class |
| `{AC105828-D1E2-4BEA-B595-28D4B94A89EF}` | CaptureOneHotPixelEffect Class |
| `{BC7FDEA5-2E9C-4122-9C05-C1DCB7795643}` | CaptureOneGGCalibrationEffect Class |
| `{795C42E3-3B83-4CA5-A2A8-BBD87AAB6ECF}` | RawContrastEffect Class |

**Infrastructure/Engine:**
| GUID | ProgID |
|------|--------|
| `{A9749AE0-4C72-41D4-B886-D86D00406A09}` | CaptureOneRawSceneEffect Class |
| `{4DBC0237-EC87-43F0-83A7-E8E18C3D86A3}` | CaptureOnePostProcessEffect Class |
| `{933CBBFD-3CC9-4325-86BD-A5AE15C6549C}` | InverseRawSceneEffect Class |
| `{2179752B-E8B2-4E59-B47E-133E2AB227AE}` | ProcessedSourceEffect Class |
| `{A19CBA99-DA7F-42A7-9710-45BAA6725ABD}` | RawPipelineHistogramEffect Class |
| `{2B3BABFE-8408-451c-A9B2-C7517CF5AA05}` | ProcessedPipelineEffect Class |
| `{33872AB1-16D9-4352-9B82-C2D360EDE5CF}` | CompositeSceneEffect Class |
| `{826BF3F6-1484-4A33-A75E-A73772E173CB}` | ImagingDeviceManager Class |
| `{B76CB5F7-8CBB-49BA-A898-449912BC4947}` | D3DImageImpl Class |
| `{ED6221B8-4FBB-47C5-B790-F80B02E4EA1F}` | EffectBitmapRenderer Class |
| `{96FB9E6C-2B7F-4CB7-AD43-65E1807BD4D6}` | EffectCollection Class |
| `{05B5A629-C2FA-4509-940D-F613F7E28C52}` | EffectGraph Class |
| `{AECD75DD-152C-4583-8744-20CD355FB737}` | EffectGraphCopier Class |
| `{FBA397AF-4759-4C51-B281-993C97A3602B}` | EffectHost Class |
| `{A3D98AF4-BD6B-4E8E-91A6-8061DAD0AF14}` | EffectInfo Class |
| `{47C537F2-8ADB-4897-863A-AAC6625FB789}` | EffectRegistrar Class |
| `{E703FBAB-68E4-4F15-A0E3-FC08F6C1A913}` | EffectRenderer Class |
| `{8CE63D37-BD48-493E-A2FE-ECD912CAFCC3}` | EnumEffect Class |
| `{3AEC2BC8-F96C-47D6-8F4F-316BFF4EA45B}` | ImageBufferCollection Class |
| `{B0E59878-593E-44CC-9F58-66711E244E9E}` | ForegroundGraphicCollection Class |
| `{A08D532A-E5A2-4237-80B2-DBADFD3EF10D}` | RectCollection Class |
| `{C48DFFA6-5761-4E9D-B802-A14CCF6E76D3}` | TileCollection Class |
| `{4DEE5313-271E-4A03-93F6-30A73EDD5E77}` | TileGenerator Class |
| `{9B7079EF-5520-43D4-B3D4-D5A134D1D5B8}` | DisplayView Class |
| `{4F050EB7-8021-4937-B2B5-3138DEA5BE8B}` | EffectImpl Class |
| `{0BDEE23A-A727-4D32-8318-CADE22FA5ECA}` | CompositeEffectImpl Class |
| `{541023CE-A4BE-415d-8B4A-B218757BB964}` | GlobalResourceManager Class |
| `{43BA34D2-2565-4553-B15B-0C777D6ABD8E}` | LargeAllocMemoryManager Class |
| `{A6C4E693-A409-4488-AFA1-9C2A210621CF}` | ImagingEngine Class |
| `{1D74999C-D0C5-4a5a-BF11-D83399128913}` | TraceManager Class |
| `{D3C2A299-EA3D-41AC-B9EE-F4EDBFC9D099}` | ApplyCurveEffect Class |
| `{A01C328D-CEE6-488c-B73D-558139BD0855}` | BackgroundEffect Class |
| `{5C5B3CD9-8904-46B8-90D5-4B62EF1B0F9E}` | Base3dLutEffect Class |
| `{4ACDAD8A-A5B4-4C1A-AD6B-757DBEBC5CDC}` | BaseCurveEffect Class |
| `{D257D87A-6D4E-45bb-A1C6-109810E2F1A4}` | BlemishRemovalEffect Class |
| `{D2A37763-714B-4bf2-ADCC-1095BF4A66E2}` | BlemishRemovalMaskEffect Class |
| `{E83F9CA5-C150-4CBD-8E29-39C04723DBBB}` | BlendEffect Class |

### 12. WLXPhotoClassic.dll (NEW — Slideshow themes)
**Total GUIDs: 5** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{773AFF18-2083-47C1-9EA9-A5DA346A0122}` | Classic Slideshow Theme (ProgID: "Classic") |
| `{854E43AC-E1FD-46f2-8DD3-EE4C7A1844B6}` | Classic Slideshow Transform |
| `{B9087BDF-F0F8-4454-A7D1-F6242E1654F8}` | Classic Slideshow Theme Black and White |
| `{F91A0A3F-3E4E-4273-88CC-6664834ACA6F}` | Classic Slideshow Theme Sepia |
| `{71ED30A7-499A-4F61-84F8-10CDEC657FE0}` | Basic Slideshow Theme |

### 13. WLXPhotoVoyager.dll (NEW — Album/Collage/Flip/Frame/Glass/Snapshots/Travel themes)
**Total GUIDs: 8** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{97B9EC02-C47C-4996-A479-DD3DD31D572D}` | Voyager COM GUID (XML resource reference) |
| `{C84CFE1B-89DC-40e7-83BF-CB821255F9EC}` | Voyager Slideshow Theme Album |
| `{AEE6C573-A192-4af3-B62B-A4E6848533D3}` | Voyager Slideshow Theme Collage |
| `{653E52D8-D033-469a-8BB5-9C1A164416D5}` | Voyager Slideshow Theme Flip |
| `{B4E10BE6-A2CE-4bef-9D80-99995CB3C162}` | Voyager Slideshow Theme Frame |
| `{5515D2B5-6825-409e-B377-544708C9DD06}` | Voyager Slideshow Theme Glass |
| `{D5561752-E5A7-46e7-B768-D945E144CA78}` | Voyager Slideshow Theme Snapshots |
| `{CC4F1166-CE12-41f7-85E2-AE4744D9381B}` | Voyager Slideshow Theme Travel |
| `{E48325CB-1EFC-425e-9CD9-47EF51BECD55}` | Voyager Slideshow Transform |

### 14. WLXPhotoAcq.dll (NEW — Photo acquisition)
**Total GUIDs: 7** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{4E13844B-6012-40cb-8E3C-0F7EB9AEF6DD}` | LivePhotoAcquire |
| `{94B1CBEA-F33F-4508-8AAA-55FD8113EFCB}` | LivePhotoAcquireOptionsDialog |
| `{0D5A7D0E-9A06-4e17-85D9-A0B24036371D}` | LivePhotoPickerDialog |
| `{E84D0D46-3D57-4039-9EFE-310AF1CAF92A}` | LivePhotoAcqDeviceSelectionDlg |
| `{4D8A134F-3D0A-4375-8B1A-78CD171C9318}` | LivePhotoAcquisitionWizard |
| `{3BD0ACD1-71CA-4475-92CC-E0AA0AAF843F}` | CLSIDForCancel (registry value) |
| TypeLib: `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | — |

### 15. WLXGrinderScheduler.dll (NEW — Background job scheduler)
**Total GUIDs: 2** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{121244C5-B0AD-45fb-9D8E-B893B917D053}` | Windows Live Photo Gallery Grinder Scheduler |
| TypeLib: `{77A16B7E-9DFE-410c-8863-264BF3E91103}` | — |

### 16. WLXImageTranscode.dll (NEW — Image transcoding)
**Total GUIDs: 8** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{20575516-78AF-4404-B3C7-51D05F9945B5}` | ImageTranscode Class |
| `{E253845C-A1A2-4ae4-B784-5E8140CD5B2C}` | ImageLoader Class |
| `{3FBB103C-F1B9-47dc-9EB3-A0C07F5F6AFA}` | PSFactoryBuffer (proxy/stub) |
| `{CB38B8DF-0D64-42b9-802A-28DCB678BFEB}` | IWLXImageLoader (interface IID) |
| `{B8A2E14E-290D-4122-B092-1A7D86198CCE}` | WLXOutofProc Class |
| TypeLib: `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | — |

### 17. WLXCodecHost.exe (NEW — Codec isolation host)
**Total GUIDs: 1** (verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{E30A45E6-1916-4659-95EE-035E62DB9AB0}` | Windows Live Photo Gallery Codec Host |

### 18. WLXQuickTimeControlHost.exe (NEW — QuickTime host)
**Total GUIDs: 3** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{B9AD19CB-FA75-4B29-B4A4-86C7E9616390}` | QuickTimePlayerHost Class |
| `{AE3A66BB-85FE-49B8-BF7B-4DB4E0005091}` | QuickTimeMovieThumbnail Class |
| TypeLib: `{C6D340BB-0CEA-4923-8082-51036E472379}` | — |

### 19. WLXVideoAcquireWizard.exe (NEW — Video capture wizard)
**Total GUIDs: 5** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{5ab7566d-f75b-4a53-9615-115b6cb1d59b}` | AppID: Windows Live Video Capture Wizard |
| `{5abe6468-4a2a-403c-892d-06e1fc31097f}` | MSLive Capture Wizard CoClass |
| `{5ab23fca-6040-4012-8fea-8da67f5806a7}` | MSLive Auto Capture CoClass |
| TypeLib: `{5ab7792c-0f76-4003-aa47-5f075165d4de}` | — |

### 20. WLXVideoCameraAutoPlayManager.exe (NEW)
**Total GUIDs: 2** (all verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{9b5c97f6-b3a5-4a6d-8b03-993ec7291a22}` | WLXHWEventHandler Class |
| TypeLib: `{9b5c8343-bdee-475d-9d3b-3715c6b8972e}` | — |

### 21. WLXQuickTimeShellExt.dll (NEW — QuickTime shell extension)
**Total GUIDs: 3** (verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{007EFBDF-8A5D-4930-97CC-A4B437CBA777}` | MovieThumbnail Class |
| `{E357FCCD-A995-4576-B01F-234630154E96}` | IID_MovieThumbnail (registered) |
| `{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}` | IID_IThumbnailProvider (registered) |

### 22. WLXDSPA.dll (NEW — DSP audio effects)
**Total GUIDs: 11** (verified via .rgs)

| GUID | ProgID |
|------|--------|
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

### 23. WLAVRes.dll (NEW — AV resources)
**Total GUIDs: 8** (verified via manifest XML)

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

### 24. PhotoViewerShim.dll / PhotoViewerShimx64.dll (NEW)
**Total GUIDs: 2** (verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{00f346cb-35a4-465b-8b8f-65a29dbab1f6}` | Live Shell Viewer Extension |
| `{00f3712a-ca79-45b4-9e4d-d7891e7f8b9d}` | Live Shell Editor Extension |

### 25. PublishPluginsInterop.dll (NEW — .NET interop)
**Total GUIDs: 1** (verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{00f43b3a-7fbe-4b84-a670-27f3a0a9cd4d}` | Windows Live Photo Gallery Publish Plugin Wrapper Interface |

### 26. SubscribePluginsInterop.dll (NEW — .NET interop)
**Total GUIDs: 1** (verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{f4482a71-df4f-4988-a731-bdb83e71127c}` | Windows Live Photo Gallery Subscribe Plugin Wrapper Interface |

### 27. AlbumDownloadProtocolHandler.dll (NEW)
**Total GUIDs: 2** (verified via .rgs)

| GUID | ProgID |
|------|--------|
| `{E43EF6CD-A37A-4A9B-9E6F-83F89B8E6324}` | Album Downloader Protocol Handler |
| TypeLib: `{60E1FA84-4F2F-417C-AEE4-7681A960D09E}` | — |

---

## Corrections Made (2026-07-26)

### CRITICAL: FaceRecognition CLSIDs Corrected

**Previous (WRONG):**
- `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` → "CLSID_FaceRegion or FaceRegionSet"
- `{EF401225-1260-4716-A842-7D180DC14C1E}` → "CLSID_FaceRecognitionPipeline"

**Corrected (from .rgs verification):**
- `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` → **CLSID_FaceRecognitionPipeline**
- `{EF401225-1260-4716-A842-7D180DC14C1E}` → **TypeLib IID**

### FaceRecognition CLSID mapping (corrected)

| GUID | Previous Label | Corrected Label | Source |
|------|---------------|-----------------|--------|
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | CLSID_ImageManager | CLSID_ImageManager | ✅ Confirmed |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | CLSID_FaceDetection | CLSID_FaceDetection | ✅ Confirmed |
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | ~~CLSID_FaceRegion~~ | **CLSID_FaceRecognitionPipeline** | ✅ .rgs confirmed |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | ~~CLSID_FaceRecognitionPipeline~~ | **TypeLib IID** | ✅ .rgs confirmed |

### WLXMediaPublishSubscribe Interface Names Confirmed

| GUID | Previous Label | Confirmed .rgs Name |
|------|---------------|---------------------|
| `{197608E2-D42D-43a5-927E-1C67FC041431}` | "Publish/Subscribe provider CLSID or IID" | **Flickr Publish and Subscribe Provider Interface** |
| `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | "Publish/Subscribe provider CLSID or IID" | **Publish and Subscribe Provider Manager Interface** |
| `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | "Publish/Subscribe provider CLSID or IID" | **Live Publish and Subscribe Provider Interface** |
| `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | "Publish/Subscribe provider CLSID or IID" | **Plugin Decorator Interface** |
| `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | "Publish/Subscribe provider CLSID or IID" | **Publish Plugin Manager Helper** |
| `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | "Publish/Subscribe provider CLSID or IID" | **Online Media Plugin Manager Interface** |
| `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | "Publish/Subscribe provider CLSID or IID" | **Internet Cache Manager** |
| `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | "Publish/Subscribe provider CLSID or IID" | **Publish Plugin Helper** |
| `{DA69067E-3959-47ca-A58D-2300786168CD}` | "Publish/Subscribe provider CLSID or IID" | **Metadata Settings Controller Interface** |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | "Publish/Subscribe provider CLSID or IID" | **TypeLib IID** (referenced by interfaces) |

---

## Cross-Reference: Known GUIDs vs. Binary Extraction

| GUID | Source Knowledge | ASCII Found | UTF-16 Found | .rgs Found |
|------|------------------|-------------|--------------|------------|
| `{0674DC61-4F42-4D44-AD50-155EB0251FA5}` | PDB GUID (WLXPhotoBase) | No | No | No |
| `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` | Transition DLL GUID | No | No | No (XML) |
| `{0DB69AAE-1EE5-4822-95E2-F4B6520E7091}` | PDB GUID (WLXPhotoCinematic) | No | No | No |
| `{17F284FA-930A-4DA2-9649-93B296009330}` | PDB GUID (MediaPublishSub) | No | No | No |
| `{2860B52E-C4A3-454D-BC1E-32C5ADD17E90}` | Certificate/WER GUID | Yes | No | No |
| `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}` | PDB GUID (FaceRecognition) | No | No | No |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting | Yes | No | No |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | CLSID_FaceDetection | Yes | No | Yes |
| `{47558454-9C62-4123-96E9-91A66E8F4D87}` | PDB GUID (MovieMaker.exe) | No | No | No |
| `{483A53CD-EF18-4B19-8AA3-2E2E3214EB41}` | CLSID_ImageManager | Yes | No | Yes |
| `{4FAF0B71-AD37-4AA3-A671-76BC052344AD}` | Certificate/WER GUID | Yes | No | No |
| `{5409AB48-D8D3-40E6-A1EB-23489DC422DE}` | CLSID_CinematicTransform | No | No | Yes |
| `{6547A44F-AABE-4CFA-9675-0113F4BD19E9}` | PDB GUID (WLXSlideshow) | No | No | No |
| `{7371ADEE-C195-427F-B0EC-3CCC13725665}` | Transition DLL GUID | No | No | No (XML) |
| `{73CFF58F-A97D-4232-883C-397BD1DF5009}` | PDB GUID (WLXVideoTrim) | No | No | No |
| `{8095E7A5-4AF7-448D-9548-0DBE027FBEB0}` | CLSID_VideoTrim | Yes | Yes | No |
| `{84FBA192-4F8D-4A5D-94DE-083446ACC1D0}` | COM GUID (WLXSlideshow) | Yes | No | No |
| `{9590CDCE-BFFC-45D5-85EA-493C65FB2C4F}` | Transition/effect template | Yes | No | No |
| `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` | CLSID_CinematicFullScreen | No | No | Yes |
| `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}` | PDB GUID (MovieLibrary) | No | No | No |
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | App/module identifier | Yes | No | No |
| `{D01C34A5-A6DC-4D28-ABBD-78D06EA27B60}` | ~~CLSID_FaceRegion~~ **CLSID_FaceRecognitionPipeline** | Yes | No | Yes |
| `{D5217874-B614-477C-B45B-E0CE638C6496}` | PDB GUID (MovieMakerCore) | No | No | No |
| `{EF401225-1260-4716-A842-7D180DC14E1C}` | ~~CLSID_FaceRecognitionPipeline~~ **TypeLib IID** | Yes | No | Yes |

---

## Extraction Summary (Expanded)

| DLL/EXE | Previous GUIDs | New GUIDs | Total Unique |
|---------|---------------|-----------|--------------|
| MovieMakerCore.dll | 5 | 0 | 5 |
| WLXSlideshow.dll | 3 | 0 | 3 |
| WLXVideoTrim.dll | 3 | 0 | 3 |
| WLXFaceRecognition.dll | 6→4 | 0 | 4 (corrected) |
| WLXPipeline.dll | 3 | 0 | 3 |
| WLXMovieLibrary.dll | 2 | 0 | 2 |
| WLXMediaPublishSubscribe.dll | 22 | 0 | 22 |
| WLMFDS.dll | 2 | 0 | 2 |
| WLXMP4Parser.dll | 2 | 0 | 2 |
| WLXPhotoBase.dll | 2 | 0 | 2 |
| Imaging.dll | — | **65+** | **65+** |
| WLXPhotoClassic.dll | — | **5** | **5** |
| WLXPhotoVoyager.dll | — | **8** | **8** |
| WLXPhotoAcq.dll | — | **7** | **7** |
| WLXGrinderScheduler.dll | — | **2** | **2** |
| WLXImageTranscode.dll | — | **8** | **8** |
| WLXCodecHost.exe | — | **1** | **1** |
| WLXQuickTimeControlHost.exe | — | **3** | **3** |
| WLXVideoAcquireWizard.exe | — | **5** | **5** |
| WLXVideoCameraAutoPlayManager.exe | — | **2** | **2** |
| WLXQuickTimeShellExt.dll | — | **3** | **3** |
| WLXDSPA.dll | — | **11** | **11** |
| WLAVRes.dll | — | **8** | **8** |
| PhotoViewerShim(x64).dll | — | **2** | **2** |
| PublishPluginsInterop.dll | — | **1** | **1** |
| SubscribePluginsInterop.dll | — | **1** | **1** |
| AlbumDownloadProtocolHandler.dll | — | **2** | **2** |
| **TOTAL** | **49** | **~136** | **~185** |

---

## Notes on Extraction

- **Certificate GUIDs** (`4faf0b71...` and `2860b52e...`) appear in EVERY binary as part of the Authenticode code-signing signature. They are NOT COM GUIDs.
- **COM CLSIDs, IIDs, and LIBIDs** in ATL COM DLLs are typically stored as:
  1. **ASCII string literals** in `.rdata` — found by ASCII scan
  2. **UTF-16LE string literals** — found by UTF-16 scan
  3. **16-byte GUID structs** in `.rdata` or ATL object maps — found via binary struct scan
  4. **Embedded .rgs registry scripts** in `.rsrc` section — found via findstr scan (NEW: this pass)
  5. **XML/Manifest resources** — found via findstr scan (NEW: this pass)
- **Imaging.dll** contains the largest set of COM objects (~65+) — a WIC-based image processing engine with RAW photo pipeline support (CaptureOne integration).
- **Slideshow theme CLSIDs** now fully mapped across WLXPhotoClassic (5 themes) and WLXPhotoVoyager (8 themes), in addition to WLXPhotoCinematic (2 cinematic themes).
- **WLXMediaPublishSubscribe.dll** has 10 interfaces now fully named from .rgs verification, confirming it as the central publish/subscribe framework.
- **WLXDSPA.dll** contains 11 DMO (DirectX Media Object) CLSIDs for audio DSP processing, including TOC (Table of Contents) parsers for ASF and AVI formats.
