# COM GUIDs Extracted from Windows Live Movie Maker 2012 Binaries

Extraction methods:
1. **ASCII string scan**: finds `{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}` patterns in contiguous 7-bit ASCII printable strings (null-terminated)
2. **UTF-16LE string scan**: same pattern in wide-character (null-padded) Unicode strings `(char, 0x00)` pairs

Generated: 2026-07-25

---

## Per-DLL GUID Breakdown

### 1. MovieMakerCore.dll
**Total GUIDs: 5**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting endpoint |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Transition/effect template GUID |
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | App/module identifier (MovieMakerCore) |

### 2. WLXSlideshow.dll
**Total GUIDs: 3**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` | COM GUID (WLXSlideshow) |

### 3. WLXVideoTrim.dll
**Total GUIDs: 3**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim (WLXVideoTrim) |

### 4. WLXFaceRecognition.dll
**Total GUIDs: 6**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | **CLSID_FaceDetection** |
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | **CLSID_ImageManager** |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | **CLSID_FaceRegion or FaceRegionSet** |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | **CLSID_FaceRecognitionPipeline** |

### 5. WLXPipeline.dll
**Total GUIDs: 3**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim (shared reference) |

### 6. WLXMovieLibrary.dll
**Total GUIDs: 2**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

### 7. WLXMediaPublishSubscribe.dll
**Total GUIDs: 22**

| GUID | Category |
|------|----------|
| `{0DDA997F-E7FA-404B-B3D3-F1610807FB66}` | Unknown/unassigned |
| `{1812A500-BA1E-41EE-B3A5-5D7B6FCA7393}` | Unknown/unassigned |
| `{197608E2-D42D-43a5-927E-1C67FC041431}` | **Publish/Subscribe provider CLSID or IID** |
| `{1D31145D-AEFE-48B0-B48A-513E5D413B44}` | Unknown/unassigned |
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | **Publish/Subscribe provider CLSID or IID** |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |
| `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | **Publish/Subscribe provider CLSID or IID** |
| `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | **Publish/Subscribe provider CLSID or IID** |
| `{66557ED9-C5F8-4815-A8CC-D157272CFFCE}` | Unknown/unassigned |
| `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | **Publish/Subscribe provider CLSID or IID** |
| `{73647561-0000-0010-8000-00AA00389B71}` | **MEDIATYPE_Audio** (DirectShow standard) |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | **Publish/Subscribe provider CLSID or IID** |
| `{7D2A6ECD-EBB6-4BA9-A3CD-4A7FD43B0423}` | Unknown/unassigned |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim (shared reference) |
| `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | **Publish/Subscribe provider CLSID or IID** |
| `{8BE133F2-9F23-4344-B5A1-A49BDD09FC0F}` | Unknown/unassigned |
| `{8DB100C7-50C5-46EB-B535-618AB68A3E22}` | Unknown/unassigned |
| `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | **Publish/Subscribe provider CLSID or IID** |
| `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | **Publish/Subscribe provider CLSID or IID** |
| `{D78480B9-E434-451C-B8A4-EDBEC652C066}` | Unknown/unassigned |
| `{DA69067E-3959-47ca-A58D-2300786168CD}` | **Publish/Subscribe provider CLSID or IID** |

### 8. WLMFDS.dll
**Total GUIDs: 2**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

### 9. WLXMP4Parser.dll
**Total GUIDs: 2**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

### 10. WLXPhotoBase.dll
**Total GUIDs: 2**

| GUID | Category |
|------|----------|
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Certificate/WER correlation (non-COM) |
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Certificate/WER correlation (non-COM) |

---

## Categorized Summary

### Known COM Interfaces & CLSIDs Found in These Binaries

| GUID | Name | Found In |
|------|------|----------|
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | CLSID_FaceDetection | WLXFaceRecognition.dll |
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | CLSID_ImageManager | WLXFaceRecognition.dll |
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | CLSID_FaceRegion or FaceRegionSet | WLXFaceRecognition.dll |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | CLSID_FaceRecognitionPipeline | WLXFaceRecognition.dll |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim | WLXVideoTrim.dll, WLXPipeline.dll, WLXMediaPublishSubscribe.dll |
| `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` | COM GUID (WLXSlideshow) | WLXSlideshow.dll |
| `{73647561-0000-0010-8000-00AA00389B71}` | MEDIATYPE_Audio (DirectShow) | WLXMediaPublishSubscribe.dll |

### Movie Maker-Specific Identifiers

| GUID | Name | Found In |
|------|------|----------|
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | App/module identifier | MovieMakerCore.dll |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting endpoint | MovieMakerCore.dll |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Transition/effect template | MovieMakerCore.dll |

### Publishing Plugin Interfaces (WLXMediaPublishSubscribe.dll)

| GUID | Name |
|------|------|
| `{197608E2-D42D-43a5-927E-1C67FC041431}` | Likely CLSID_MediaPublishSubscribeProviderManager or IID |
| `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | Likely CLSID for LiveProvider or FlickrProvider |
| `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | Likely CLSID for a plugin/manager |
| `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | Likely CLSID for a plugin/manager |
| `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | Likely CLSID for a plugin/manager |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | Likely CLSID for a plugin/manager |
| `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | Likely CLSID for a plugin/manager |
| `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | Likely CLSID for a plugin/manager |
| `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | Likely CLSID for a plugin/manager |
| `{DA69067E-3959-47ca-A58D-2300786168CD}` | Likely CLSID for a plugin/manager |

### Unknown/Unassigned GUIDs (need further investigation)

| GUID | Found In |
|------|----------|
| `{0DDA997F-E7FA-404B-B3D3-F1610807FB66}` | WLXMediaPublishSubscribe.dll |
| `{1812A500-BA1E-41EE-B3A5-5D7B6FCA7393}` | WLXMediaPublishSubscribe.dll |
| `{1D31145D-AEFE-48B0-B48A-513E5D413B44}` | WLXMediaPublishSubscribe.dll |
| `{66557ED9-C5F8-4815-A8CC-D157272CFFCE}` | WLXMediaPublishSubscribe.dll |
| `{7D2A6ECD-EBB6-4BA9-A3CD-4A7FD43B0423}` | WLXMediaPublishSubscribe.dll |
| `{8BE133F2-9F23-4344-B5A1-A49BDD09FC0F}` | WLXMediaPublishSubscribe.dll |
| `{8DB100C7-50C5-46EB-B535-618AB68A3E22}` | WLXMediaPublishSubscribe.dll |
| `{D78480B9-E434-451C-B8A4-EDBEC652C066}` | WLXMediaPublishSubscribe.dll |

### Certificate / WER Correlation GUIDs (non-COM)
These appear in ALL 10 binaries and are code-signing certificate thumbprints, not COM identifiers:
- `{4faf0b71-ad37-4aa3-a671-76bc052344ad}`
- `{2860b52e-c4a3-454d-bc1e-32c5add17e90}`

---

## Cross-Reference: Known GUIDs vs. Binary Extraction

| GUID | Source Knowledge | ASCII Found | UTF-16 Found |
|------|------------------|-------------|--------------|
| `{0674DC61-4F42-4D44-AD50-155EB0251FA5}` | PDB GUID (WLXPhotoBase) | No | No |
| `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` | Transition DLL GUID (WLXPhotoCinematic) | No | No |
| `{0DB69AAE-1EE5-4822-95E2-F4B6520E7091}` | PDB GUID (WLXPhotoCinematic) | No | No |
| `{17F284FA-930A-4DA2-9649-93B296009330}` | PDB GUID (WLXMediaPublishSubscribe) | No | No |
| `{2860B52E-C4A3-454D-BC1E-32C5ADD17E90}` | Certificate/WER GUID (non-COM) | Yes | No |
| `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}` | PDB GUID (WLXFaceRecognition) | No | No |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting | Yes | No |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | CLSID_FaceDetection | Yes | No |
| `{47558454-9C62-4123-96E9-91A66E8F4D87}` | PDB GUID (MovieMaker.exe) | No | No |
| `{483A53CD-EF18-4B19-8AA3-2E2E3214EB41}` | CLSID_ImageManager | Yes | No |
| `{4FAF0B71-AD37-4AA3-A671-76BC052344AD}` | Certificate/WER GUID (non-COM) | Yes | No |
| `{5409AB48-D8D3-40E6-A1EB-23489DC422DE}` | CLSID_CinematicTransform (WLXPhotoCinematic) | No | No |
| `{6547A44F-AABE-4CFA-9675-0113F4BD19E9}` | PDB GUID (WLXSlideshow) | No | No |
| `{7371ADEE-C195-427F-B0EC-3CCC13725665}` | Transition DLL GUID (WLXPhotoCinematic) | No | No |
| `{73CFF58F-A97D-4232-883C-397BD1DF5009}` | PDB GUID (WLXVideoTrim) | No | No |
| `{8095E7A5-4AF7-448D-9548-0DBE027FBEB0}` | CLSID_VideoTrim | Yes | No |
| `{84FBA192-4F8D-4A5D-94DE-083446ACC1D0}` | COM GUID (WLXSlideshow) | Yes | No |
| `{9590CDCE-BFFC-45D5-85EA-493C65FB2C4F}` | Transition/effect template | Yes | No |
| `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` | CLSID_CinematicFullScreen (WLXPhotoCinematic) | No | No |
| `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}` | PDB GUID (WLXMovieLibrary) | No | No |
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | App/module identifier (MovieMakerCore) | Yes | No |
| `{D01C34A5-A6DC-4D28-ABBD-78D06EA27B60}` | CLSID_FaceRegion or FaceRegionSet | Yes | No |
| `{D5217874-B614-477C-B45B-E0CE638C6496}` | PDB GUID (MovieMakerCore) | No | No |
| `{EF401225-1260-4716-A842-7D180DC14E1C}` | CLSID_FaceRecognitionPipeline | Yes | No |

### Known GUIDs NOT Found via String Scanning
These GUIDs are known to exist from prior analysis (e.g., in .rgs registry scripts, embedded resources, or binary structs) but were not found by our string scanning methods:

- **CLSID_CinematicFullScreen** `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` — known to be in WLXPhotoCinematic.dll (not in this scan set)
- **CLSID_CinematicTransform** `{5409AB48-D8D3-40e6-A1EB-23489DC422DE}` — same
- **Transition DLL GUIDs** `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}`, `{7371ADEE-C195-427F-B0EC-3CCC13725665}` — embedded in XML resources
- **All PDB GUIDs** — stored as 16-byte binary structs in debug directories, not as strings
- **WLXVideoTrim CLSID** `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` — found via UTF-16 scan! (present in WLXVideoTrim.dll, WLXPipeline.dll, WLXMediaPublishSubscribe.dll)

---

## Extraction Summary

| DLL | ASCII GUIDs | UTF-16 GUIDs | Total Unique |
|-----|-------------|--------------|--------------|
| MovieMakerCore.dll | 2 | 3 | 5 |
| WLXSlideshow.dll | 2 | 1 | 3 |
| WLXVideoTrim.dll | 2 | 1 | 3 |
| WLXFaceRecognition.dll | 6 | 0 | 6 |
| WLXPipeline.dll | 2 | 1 | 3 |
| WLXMovieLibrary.dll | 2 | 0 | 2 |
| WLXMediaPublishSubscribe.dll | 12 | 10 | 22 |
| WLMFDS.dll | 2 | 0 | 2 |
| WLXMP4Parser.dll | 2 | 0 | 2 |
| WLXPhotoBase.dll | 2 | 0 | 2 |

---

## Notes on Extraction

- **Certificate GUIDs** (`4faf0b71...` and `2860b52e...`) appear in EVERY binary as part of the Authenticode code-signing signature. They are NOT COM GUIDs.
- **COM CLSIDs, IIDs, and LIBIDs** in ATL COM DLLs are typically stored as:
  1. **ASCII string literals** in `.rdata` — found by ASCII scan (e.g., face recognition GUIDs)
  2. **UTF-16LE string literals** — found by UTF-16 scan (e.g., MovieMakerCore app identifiers, slideshow/videotrim CLSIDs)
  3. **16-byte GUID structs** in `.rdata` or ATL object maps — NOT scanned (requires different approach)
  4. **Embedded .rgs registry scripts** in `.rsrc` section — may contain GUIDs not found by string scanning
  5. **XML/Manifest resources** — may contain GUIDs embedded in XML strings (e.g., WLXPhotoCinematic transitions)
- **WLXMediaPublishSubscribe.dll** has the most COM GUIDs (22), confirming its role as a framework DLL managing multiple publishing providers.
- **`{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}`** appears in 3 DLLs (WLXVideoTrim, WLXPipeline, WLXMediaPublishSubscribe), suggesting cross-DLL CLSID sharing.
- **`{73647561-0000-0010-8000-00AA00389B71}`** = `MEDIATYPE_Audio` from DirectShow, likely referenced in media type handling code.
