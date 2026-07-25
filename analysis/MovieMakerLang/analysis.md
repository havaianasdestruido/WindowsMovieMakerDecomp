# MovieMakerLang.dll Analysis

## Overview
**Pure resource-only satellite DLL** containing all English (US) language/localization resources for Windows Live Movie Maker 2012. No code, no exports, no imports — only a single `.rsrc` section with UI strings, dialog templates, menu items, error messages, embedded PNG images, DirectUI UIFILE layouts, and VERSION resources.

| Field | Value |
|-------|-------|
| **File** | `MovieMakerLang.dll` |
| **Image size** | 216 KB (0x36000) |
| **Type** | PE32 x86 resource DLL |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Timestamp** | 2014-04-01 01:28:09 UTC |
| **Image base** | 0x10000000 |
| **Subsystem** | Windows GUI (2) |
| **OS version** | 6.02 (Windows 8) |
| **DLL characteristics** | Dynamic Base (ASLR), NX Compatible |
| **Build** | 16.4.3528.0331 (ship) |
| **Digital signature** | Present (Certificates Directory at RVA 0x35200, size 0x3EC8) |

## PE Structure

### File Header
- **Machine**: x86 (0x14C)
- **Number of sections**: 1
- **Characteristics**: Executable, DLL, 32-bit, Large Address Aware

### Optional Header
- **Entry Point**: 0x0000 (none — no code)
- **Size of code**: 0
- **Size of initialized data**: 0x35000 (212 KB)
- **Size of image**: 0x36000 (216 KB)
- **Checksum**: 0x45353

### Section Layout
| Section | VA | Virtual Size | Raw Size | Characteristics |
|---------|-----|-------------|----------|-----------------|
| `.rsrc` | 0x1000 | 0x34F28 (217 KB) | 0x35000 (212 KB) | Initialized Data, Read Only |

### Directory Table
| Directory | RVA | Size | Status |
|-----------|-----|------|--------|
| Export | 0 | 0 | **Empty — no exports** |
| Import | 0 | 0 | **Empty — no imports** |
| Resource | 0x1000 | 0x34F28 | All DLL content |
| Certificates | 0x35200 | 0x3EC8 | Digital signature |
| All others | 0 | 0 | Not present |

## Exports
**None.** Export directory is empty (RVA=0, size=0). This DLL has no code and no export table.

## Imports
**None.** Import directory is empty (RVA=0, size=0). This DLL has no dependencies and no IAT.

## Instancing Model
Loaded by Movie Maker (via `LoadLibrary`/`LoadString`/`FindResource`) when the UI language is set to English (US). The DLL is referenced by other components as the localized resource provider. In the source code, it would be linked as the `MOVIEMAKERLANG_DLL` `#define` and strings loaded via `LoadString(hInstLang, ...)`.

## Resource Content

The `.rsrc` section contains all standard Win32 resource types. Based on extracted strings and binary analysis:

### Resource Types Detected
- **RT_STRING** (type 6) — String tables with UI text, error messages, tooltips, accessibility labels
- **RT_DIALOG** (type 5) — Dialog templates for options, dialogs, custom settings
- **RT_MENU** (type 4) — Menu bar and context menu definitions
- **RT_ACCELERATOR** (type 9) — Keyboard accelerator tables
- **RT_VERSION** (type 16) — VERSIONINFO resource
- **RT_GROUP_ICON / RT_ICON** (types 14/3) — Application icons
- **RT_HTML / custom** — Embedded UIFILE XML for DirectUI ribbon and chrome
- **PNG images** — Embedded PNG resources (created with Adobe Photoshop CS5 / ImageReady)

### Version Info (RT_VERSION)
- **CompanyName**: Microsoft Corporation
- **FileDescription**: MovieMakerLang
- **FileVersion**: 16.4.3528.0331
- **InternalName**: MovieMakerLang
- **LegalCopyright**: (c) 2012 Microsoft Corporation. All rights reserved.
- **OriginalFilename**: MovieMakerLang.DLL
- **ProductName**: Movie Maker
- **ProductVersion**: 16.4.3528.0331
- **Language**: 0x0409 (English - US)

### DirectUI UIFILE Resources
Contains `UIFILE` resource entries used by **UXCore.dll**-based DirectUI framework. The string "SUNDANCE_RIBBON" appears, confirming ribbon layout definitions are embedded here.

### Embedded PNG Images
Multiple PNG images are embedded as resources, created with **Adobe Photoshop CS5 Windows** (Adobe ImageReady). These are toolbar icons, splash screen, and branding graphics. XMP metadata shows:
- CreatorTool: `Adobe Photoshop CS5 Windows`
- OriginalDocumentID: `uuid:FFE4F32D8833DE1194BDBE701BF9AFA3`

## COM GUIDs

The following GUIDs were found as string literals in the binary:

| GUID | Likely Purpose |
|------|---------------|
| `{1D31145D-AEFE-48B0-B48A-513E5D413B44}` | Plugin/service identifier |
| `{66557ED9-C5F8-4815-A8CC-D157272CFFCE}` | Plugin/service identifier |
| `{8BE133F2-9F23-4344-B5A1-A49BDD09FC0F}` | In context of upload/publish limitations |
| `{0DDA997F-E7FA-404B-B3D3-F1610807FB66}` | Unknown (binary GUID) |
| `{1812A500-BA1E-41EE-B3A5-5D7B6FCA7393}` | Unknown (binary GUID) |

These are likely CLSIDs for COM services or plugin identifiers referenced in error messages and resource strings.

## Extracted UI Strings (Categorized)

### Application & Window Titles
- `Movie Maker`
- `Photo Gallery Slide Show`
- `Movie Maker Options`
- `Create Custom Setting`
- `Add or Remove Effects`

### Menu & Ribbon Items
- `&New project`, `&Open project`, `&Save project`, `Save project &as`
- `&Undo`, `&Redo`, `Cu&t`, `&Copy`, `&Paste`, `Select &All`
- `&Add videos and photos`, `&Add music...`, `&Add sound...`
- `&Publish movie`, `Burn a &DVD`, `&Watch online`
- `&Credits`, `&Starring`, `&Director`, `&Location`, `Sound&track`
- `&Multiple effects...`, `&Remove`, `&Delete`
- `Abo&ut Movie Maker`, `&Options`, `Online help (F1)`

### Tooltips & Accessibility
- `Add videos and photos to the project.`
- `Add music to the project.`
- `Add an audio file to use as narration or sound effects for your project.`
- `Open the Add or Remove Effects dialog box so that you can apply more than one effect at the same time.`
- `Click here to create, open and save a project, as well as set different options for Movie Maker.`
- `Reduce the appearance of shake in the selected video clip.`
- `Play a preview of your movie full screen.`
- `Scroll`, `Line up/down/left/right`, `Page up/down/left/right`
- `Show the menu bar`, `Hide the menu bar`
- `Minimize`, `Maximize`, `Close`
- `Position`, `Press`

### Dialog Button Text
- `&Save`, `&Don't show this message again`, `Cancel`, `&Abort`, `&Retry`, `&Ignore`
- `&Yes`, `Y&es to all`, `&No`, `N&o to all`, `&Close`, `Help`
- `&Try again`, `&Continue`, `&Options`, `&Apply`
- `&Download`, `&Revert`, `&Restore project`, `&Fix item`
- `Save and Close`, `Close without Saving`, `No Thanks`

### Error Messages
- `Sorry, Movie Maker had to shut down unexpectedly. Please restart it, and then try again. (Error: 0x%1!08X!)`
- `Movie Maker can't use the command line.`
- `Movie Maker doesn't support the selected file type.`
- `Sorry, Movie Maker can't open this project file. The file might not be compatible with Movie Maker or be corrupted.`
- `Movie Maker can't find the media file named "%2".`
- `Movie Maker can't use the media file. The file might require a codec that's not installed on this computer or be corrupted.`
- `Files can't be added directly from a network folder into Movie Maker.`
- `Files can't be added directly from a removable drive into Movie Maker.`
- `Movie Maker can't use files that are protected with digital rights management.`
- `Files can't be added directly from the Internet.`
- `The required codec for the audio in the file isn't installed on your computer.`
- `The DirectShow audio or video codec that is required for the file isn't installed on your computer.`
- `The selected file requires an older audio or video codec that is no longer supported.`
- `An MPEG-2 codec needs to be installed in order to use this file.`
- `Sorry, Movie Maker can't start. Make sure your computer meets the minimum system requirements...`
- `Movie Maker can't publish your movie on the selected site. Make sure that the plug-in is installed.`
- `This file might be too large to successfully upload to %s. You can try shortening it...`
- `Your movie wasn't published successfully. Sorry, please try again later.`
- `Sorry, the service is not currently available.`
- `Sorry, you don't have the appropriate permissions to publish to the location.`
- `You don't have permission to save to this location.`
- `Contact the administrator to obtain permission.`
- `A problem has occurred. There are missing or unusable files in your Movie Maker project...`
- `Sorry, this is taking longer than expected. Do you want to cancel and close Movie Maker?`
- `Sorry, your movie can't be saved. Free some space on the disk drive...`

### Webcam / Capture / Narration
- `Select an audio device and webcam`
- `Available webcams:`, `Available audio devices:`
- `Webcam preview`, `Webcam recording indicator`
- `Start recording video`, `Stop recording video`
- `Start recording narration`, `Stop recording narration`
- `No webcam was found.`, `No audio device was found.`
- `Recording has stopped because the maximum time for recording has been reached.`
- `Capture %1!d! (%2!s! %3!s!)`
- `Maximum Resolution:`, `List of resolutions`

### Publish / Share / Save Settings
- `Recommended for this pro&ject`
- `For &high-definition display`, `For &computer`, `Burn a &DVD`, `For &email`
- `Windows Phone (lar&ge)`, `&Windows Phone (small)`
- `Apple iPh&one`, `Apple &iPad`
- `Android Phone (&medium)`, `Android Pho&ne (large)`, `Android (&1080p)`, `Android (72&0p)`
- `Windows 7 (7&20p)`, `Windows &8 (1080p)`
- `Zune HD (for &720p display)`, `&Zune HD (for device)`
- `Feature Phone (sma&ll)`, `&Feature Phone (large)`
- `&Audio only`
- `OneDrive`, `Faceboo&k`, `YouT&ube`, `Flick&r`, `&Vimeo`, `&Blip.TV`, `MyS&pace`, `DailyMo&tion`

### Pan & Zoom / Effects (transition names)
- Numerous cinematic effects: `Cinematic`, `Cinematic - blur`, `Cinematic - burst 1/2`, etc.
- Contemporary transitions: `Contemporary - drop down 1/2`, `Contemporary - fade 1/2/3`, `Contemporary - fly in left 1/2/3`, `Contemporary - slide down narrow/wide 1/2/3`
- Classic transitions: `Fade`, `Dissolve`, `Wipe`, `Slide`, `Zoom`, `Circle`, `Diamond`, `Heart`, `Star`, `Wheel`, `Iris`, `Crossfade`, `Checkerboard`, `Flip`, `Roll`, `Swing`, `Warp`, `Blur`, `Split`, `Page curl`
- Pan/zoom movements: `Pan down/up/left/right`, `Zoom in/out center/top/bottom/left/right`, `Zoom in center and rotate left/right`

### Credit Templates
- `DIRECTED BY` / `[Enter your name here]`
- `STARRING` / `[Enter your cast here]`
- `FILMED ON LOCATION` / `[Enter location here]`
- `SOUNDTRACK` / `[Enter soundtrack info here]`

### UI Element Labels (for UI Automation / Accessibility)
- `Timeline Pane`, `Storyboard`, `Thumbnail size`, `Preview`, `Change thumbnail size`
- `Text Box`, `Text Track`, `Music Track`, `Visuals Track`, `Audio Track`, `Captions`
- `Progress bar`, `Progress Context`, `Progress Details`, `Statusbar Progress`, `Statusbar Progress Cancel`
- `Playback indicator`, `Preview Scrub Slider`, `Preview Position Label`, `Preview Duration Label`
- `Waveform group`, `Waveforms`, `Time Zoom`, `Zoom time scale`
- `Trim markers`, `Trim tool`, `Trimmed Duration Label`, `Trimmed duration: %1`

### File Type Descriptions
- `Movie Maker Project` (.wlmp)
- `Movie Maker Video Settings File` (.wlvs)
- `Windows Media Audio File`
- `Windows Media Video File`
- `MPEG-4/AAC Audio File`
- `MPEG-4/H.264 Video File`
- `All Files`

### Ribbon / Tab Labels
- `Home`, `Animations`, `Visual Effects`, `Project`, `View`, `Format`, `Edit`
- `Video Tools`, `Music Tools`, `Narration Tools`, `Text Tools`, `Captions`
- `Audio and Video`, `Audio and Music`
- `General tab`, `Advanced tab`, `Captions tab`, `Trust Center tab`, `Audio and Video tab`

### Miscellaneous
- `Segoe UI` (font family used throughout UI)
- `Windows Live ID`
- `http://g.msn.com/5meen_us/122` (help URL)
- `%1!s! kbps, %2!s! kHz, mono`, `%1!s! kbps, %2!s! kHz, stereo`
- `Display size: %1!u! x %2!u! pixels`
- `Aspect ratio: widescreen`, `Aspect ratio: standard`
- `Estimated movie file size: %1!s! MB`, `Free disk space: %2!s! MB`
- `Version 2012 (Build %s)`
- `Portions of this software are based in part on the work of the Independent JPEG Group.`
- `? 2012 Microsoft Corporation. All rights reserved.`
- `Allowing Microsoft to collect data about your computer and how you use applications helps us improve...`
- `Loading audio data`, `Loading...`
- `Apply to all`, `Fit to music`, `Fit photos to music`

## Key Observations

1. **Pure resource satellite DLL** — No code, no entry point, no imports/exports. This is the canonical pattern for MUI (Multilingual User Interface) satellite DLLs in Windows.

2. **Single `.rsrc` section** — All 212 KB of raw data is in the resource section. No `.text`, `.data`, `.reloc`, or `.tls` sections exist.

3. **Language**: `en-us` (English - United States), locale ID 0x0409.

4. **Digital signature present** — Certificates Directory at RVA 0x35200 (0x3EC8 bytes), indicating the DLL was Authenticode-signed.

5. **DirectUI UIFILE resources** — Contains UICC-compiled UIFILE XML resources for the DirectUI-based ribbon and chrome, loaded by UXCore.dll.

6. **Ribbon architecture**: References `SUNDANCE_RIBBON` (the internal codename for Movie Maker was "Sundance").

7. **PNG images processed with Adobe Photoshop CS5** — Multiple embedded PNG toolbar/menu icons created with Photoshop CS5.

8. **No COM registration in this DLL** — Unlike other Movie Maker DLLs, this one has no COM exports (DllRegisterServer, etc.) and no self-registration. It's purely a data DLL.

9. **Targeting Windows 8** — OS version 6.02 in the PE header matches the build being designed for Windows 8.

10. **Plug-in publishing model**: References to Flickr, Vimeo, YouTube, Facebook, Blip.TV, MySpace, DailyMotion, OneDrive as publishing targets, plus `Free Music Archive` as a music source.
