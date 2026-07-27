# WLXPhotoLibraryDuiResources.dll + Localized — Analysis

Two DirectUI resource-only DLLs from Windows Essentials / Photo Gallery (v16.4.3528.0331, April 2014). Both are PE32 x86, pure `.rsrc` section (no imports, no exports, no code).

---

## WLXPhotoLibraryDuiResources.dll (Non-Localized)

**Size:** 4.33 MB | **Timestamp:** 2014-04-01 | **Description:** "Photo Gallery Non-Localized Resources"

### Resource Type Table

| Type ID | Win32 Type | Count | Description |
|---------|-----------|-------|-------------|
| PNG (named) | IMAGE | 15 | Toolbar/tab images (IDs 105–148) |
| 1 | CURSOR | 21 | Custom cursors |
| 6 | STRING | 2 | String table blocks (IDs 1310, 1318) |
| 10 | RCDATA | 91 | 2-byte layout data (coordinates/metrics) |
| 12 | GROUP_CURSOR | 18 | Cursor groups |
| 16 | VERSION | 1 | VS_VERSION_INFO |
| 4000 | DirectUI | 1432 | RLE-compressed DirectUI layout data |
| 4002 | DirectUI_0409 | 109 | DirectUI class definitions |
| 4003 | DirectUI | 34 | DirectUI binary resources |
| 4006 | REGINST | 14 | Registry install data |
| 4008 | SCHEMA | 3 | Schema definitions |
| 4009 | THEME | 3 | Theme resources |
| 4010 | UIB | 11 | **Compiled DirectUI UIFILE** (binary XML) |
| 4011 | OEM | 18 | OEM customization |
| 4012 | FONT_FALLBACK | 1014 | Font fallback tables |

### Type 4000 — RLE-Compressed DirectUI Layouts

Largest group (1432 entries). Binary-compressed with `RLE` magic header:

```
Offset 0x00: "RLE" (3 bytes magic)
Offset 0x03: Version byte (0x01)
Offset 0x04: Compressed layout data
```

Entries range from ~43 bytes to ~2 KB. These are compiled DirectUI UIFILE layouts stored in Microsoft's proprietary RLE binary encoding. The actual XML layout source is not present — only the compiled binary form.

### Type 4010 — UIB (Compiled UIFILE) Containers

11 entries. These are the primary UI layout definitions in a structured binary format:

```
Offset 0x00: "UIB\0" (4-byte magic)
Offset 0x04: Version (uint16 LE) = 0x0001
Offset 0x06: Flags (uint16 LE)
Offset 0x08: Total size (uint32 LE)
Offset 0x0C: Entry count (uint16 LE)
Offset 0x0E: Offset table (uint32 LE each)
Then: String table with null-terminated class/property names
```

**DirectUI class names found** (sampled from entry 4400):

- `Element`, `Class`, `SubHeader`, `Foreground`, `FontSize`
- `Header`, `Padding`, `Margin`, `Button`, `Metadata`
- `LockupPaneSlider`, `Slider`, `Thumb`, `RepeatButton`
- `LockupPaneSliderMore`, `LockupPaneSliderLess`
- `KeyFocused`, `ContentAlign`, `Width`, `AccDesc`, `Tooltip`
- `dmx`, `dmx1`

These define the Photo Gallery's DirectUI element tree — gallery views, sliders, thumbnails, navigation panes, and media playback controls.

### Type 4002 — DirectUI Class Definitions

109 entries. Entry ID 4180 starts with bytes `CDDD FF` — this is the DirectUI class registration table mapping class names to creation factories.

### Type 4012 — Font Fallback Tables

1014 entries. Each is a compact binary table mapping base fonts to fallback fonts for international text rendering. Entry 103 (26 bytes) is typical — these are small lookup tables.

---

## WLXPhotoLibraryDuiResourcesLocalized.dll (Localized)

**Size:** 0.99 MB | **Timestamp:** 2014-04-01 | **Description:** "Photo Gallery Localized Resources"

### Resource Type Table

| Type ID | Win32 Type | Count | Description |
|---------|-----------|-------|-------------|
| AVI (named) | VIDEO | 1 | RLE-compressed AVI video (ID 14636, 11802 bytes) |
| MUI (named) | MUI | 1 | MUI language metadata (English US, lang=1033) |
| PNG (named) | IMAGE | 93 | UI icons and images |
| UIFILE (named) | UIFILE | 0 | Declared but empty (resources in Type4000) |
| WEVT_TEMPLATE (named) | EVENT | 1 | Event manifest |
| XML (named) | XML | 0 | Declared but empty |
| 1 | CURSOR | 2 | Cursors |
| 2 | BITMAP | 11 | Toolbar/menu bitmaps |
| 3 | ICON | 41 | Application/window icons (multiple sizes) |
| 4 | MENU | 9 | Context menus |
| 5 | DIALOG | 20 | Settings/property dialogs |
| 6 | STRING | 251 | **Localized UI strings** (English) |
| 9 | ACCELERATOR | 11 | Keyboard shortcuts |
| 12 | GROUP_CURSOR | 2 | Cursor groups |
| 14 | GROUP_ICON | 5 | Icon groups |
| 16 | VERSION | 1 | VS_VERSION_INFO |
| 4000 | DirectUI | 28 | PNG images (misnamed as DirectUI type) |
| 4009 | THEME | 21 | Theme pointer entries (4 bytes each) |

### String Resources (Type 6)

251 string table blocks containing all English UI text for Photo Gallery. Key categories:

- **File operations:** "Email photos and videos", "Publish on &Flickr...", "Save as..."
- **Tags and metadata:** "Enter a tag here", "Flag photos and videos"
- **Face detection:** "Face detection and recognition"
- **View controls:** "Fit to window", "Enter a tag here"
- **Error messages:** "Sorry, no items matched your search", disk space warnings
- **Panorama/Photo Fuse:** "Restart Photo Gallery and try again..."
- **Account/access:** "The access to this account is denied"

### AVI Resource

Contains a RLE-compressed AVI video (ID 14636). Likely a UI animation (loading spinner, transition effect).

### MUI Resource

Standard MUI (Multilingual User Interface) metadata declaring:
- Supported resource types: AVI, PNG, UIFILE, WEVT_TEMPLATE, XML, MUI
- Language: `en-us` (English United States)

### Type 4009 — Theme Entries

21 entries, each 4 bytes. Two patterns observed:
- `01 00 60 80` / `01 00 60 00` — Theme resource pointers (DPI-aware flag at bit 7 of byte 3)
- `03 00 6F 4B` / `03 00 E9 4E` / `03 00 04 4F` — Alternative theme references

### Type 4000 (DLL2) — PNG Images

All 28 entries in this type are actually PNG images (starting with `89 50 4E 47`), not DirectUI layouts. This is a naming collision — the DLL uses the DirectUI type ID range for image storage.

---

## Architecture Summary

```
Photo Gallery.exe
├── WLXPhotoLibraryDuiResources.dll (non-localized)
│   ├── 1432 RLE-compressed DirectUI layouts (Type4000)
│   ├── 11 compiled UIFILE containers with class/property names (Type4010)
│   ├── 109 DirectUI class definitions (Type4002)
│   ├── 1014 font fallback tables (Type4012)
│   ├── 91 RCDATA layout metrics (Type10)
│   └── 15 PNG toolbar images
│
└── WLXPhotoLibraryDuiResourcesLocalized.dll (localized)
    ├── 251 string table blocks (all English UI text)
    ├── 41 icons, 11 bitmaps, 9 menus, 20 dialogs
    ├── 93 PNG UI images
    ├── 11 keyboard accelerators
    ├── 21 theme entries
    ├── 1 AVI animation
    └── MUI language metadata (en-us)
```

## Key Findings

1. **DirectUI UIFILE layouts are compiled to binary** — both `RLE`-compressed (Type4000) and `UIB` container format (Type4010). No plain-text XML UIFILE source exists in these DLLs.

2. **UIB format contains class/property names** — the compiled UIFILE embeds DirectUI class names (Element, Button, Slider, etc.) as a string table, enabling runtime element tree construction.

3. **Localization is string-only** — all UI layout definitions are in the non-localized DLL; the localized DLL contains only string tables, standard Win32 resources (icons, menus, dialogs), and theme metadata.

4. **Type4012 (1014 entries) is the largest resource group** — font fallback tables dominate the non-localized DLL, supporting international text rendering.

5. **MUI framework integration** — the localized DLL uses standard Windows MUI metadata for language detection and resource loading.
