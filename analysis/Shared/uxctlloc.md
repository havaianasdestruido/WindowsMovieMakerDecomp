# UXCtlLoc.dll — Localized Controls Resource DLL

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Timestamp | Tue Apr 1 01:27:28 2014 (0x533A4030) |
| Image Base | 0x10000000 |
| Size of Image | 0x6000 (24 KB) |
| Sections | 1 — `.rsrc` only |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible, **No SEH** |
| Entry Point | None (0x00000000) |
| Code Size | 0 |
| Resources | 0x40F8 (~16 KB) |

## Exports
**None** — This DLL has no exports.

## Imports
**None** — This DLL has no imports.

## Structure
This is a **pure resource-only DLL** — it contains only a `.rsrc` section with no code, no data, and no imports.

### Resource Content
The 16 KB resource section contains localized UI definitions for Windows Live shared controls. Based on the naming convention (`uxctlloc` = UX Controls Localized), this holds:
- UIFILE templates (localized versions of the controls in uxctl.dll)
- String tables for different languages
- Possibly localized images/icons

## Architecture
1. **Satellite DLL pattern** — Windows Live apps load this DLL at runtime based on the user's UI language
2. **No code** — purely resource data, loaded via `LoadLibrary` + `FindResource`/`LoadResource`
3. **Companion to uxctl.dll** — contains the locale-specific resources that uxctl.dll references via the RM (Resource Manager) API
4. **Small footprint** — only 24 KB image size, suggesting it holds resources for a single locale or a small set of shared strings
5. **The RM API** in uxctl.dll (`RMInitialize`, `RMUpdateResourceSet`) likely loads this DLL and its resources into the resource lookup chain

## Relationship to Other DLLs
- **uxctl.dll** references this for localized content
- **uxcore.dll** provides the `CRMResource`, `CRMStringResource`, `CRMColorResource` classes that ultimately load from resource-only DLLs like this one
