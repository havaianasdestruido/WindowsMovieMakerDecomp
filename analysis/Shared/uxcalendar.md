# UXCalendar.dll — Windows Live Calendar UI

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Timestamp | Tue Apr 1 01:26:12 2014 (0x533A3FE4) |
| Image Base | 0x10000000 |
| Size of Image | 0x1C000 (112 KB) |
| Code Size | 0x15400 (~85 KB) |
| Subsystem | Windows GUI (2) |
| PDB | `UXCalendar.pdb` — GUID `{3B852645-1902-4FFF-A8FA-162A502B2596}` |
| C/C++ Functions | 37 |

## Exports (33 total)
### Core API
| Ordinal | Name | Description |
|---------|------|-------------|
| 23 | `UXCalendarInitProcess` | Initialize calendar (takes IUnknown + HINSTANCE) |
| 24 | `UXCalendarUnInitProcess` | Uninitialize calendar |
| 22 | `UXAttachCalStringLoader` | Attach custom string loader (IUXCalStringLoader) |
| 28 | `UXReleaseCalStringLoader` | Release string loader |

### Date Formatting
| Ordinal | Name | Description |
|---------|------|-------------|
| 1 | `CompareISODates` | Compare two ISO date strings → result int |
| 2 | `CurrentTime` | Get current time as ISO string |
| 5 | `FormatCurrentDate` | Format current date |
| 6 | `FormatCurrentDateLocalized` | Format current date, locale-aware |
| 7 | `FormatDate` (int version) | Format date from int |
| 8 | `FormatDate` (string version) | Format date from ISO string |
| 10 | `FormatDateLocalized` (double) | Format localized date from variant time |
| 11 | `FormatDateLocalized` (string) | Format localized date from ISO string |
| 12 | `GetLocalizedDay` | Get localized day name |
| 25 | `UXGetLocalizedDay` | Get localized day (alternate API) |

### Date Parsing
| Ordinal | Name | Description |
|---------|------|-------------|
| 13 | `GetVariantTime` | Convert ISO string → variant time (double) |
| 14 | `IsValidDate` | Validate ISO date string |
| 15 | `NumberOfDays` | Calculate days between two dates |
| 16 | `NumberOfHours` | Calculate hours between two dates |
| 17 | `ParseISOtoSystemTime` (string) | Parse ISO string → SYSTEMTIME |
| 18 | `ParseISOtoSystemTime` (buffer) | Parse ISO buffer → SYSTEMTIME |
| 26 | `UXParseUserDateString` | Parse user-entered date string |
| 27 | `UXParseUserTimeString` | Parse user-entered time string |

### Calendar Utilities
| Ordinal | Name | Description |
|---------|------|-------------|
| 3 | `DateUtils_GetFirstDayOfWeek` | Get first day of week for locale |
| 4 | `DateUtils_RemapLocaleToGregorian` | Remap non-Gregorian calendar to Gregorian |
| 9 | `DateUtils_GetFirstDayOfWeekEnd` | Get last day of week for locale |

### DirectUI Integration
| Ordinal | Name | Description |
|---------|------|-------------|
| 19 | `ReparseDateTimeFields` | Re-parse date/time fields in a DirectUI element tree |
| 20 | `ShowPopUpDatePicker` | Show popup date picker (returns via IUXCalendarPopupCallback) |
| 21 | `ShowPopUpDatePickerWithFreeBusy` | Show date picker with free/busy info |

### Event
| Ordinal | Name | Description |
|---------|------|-------------|
| 29 | `ViewChange@MiniMonthElement@@2PAEA` | View change event for mini month calendar |

## DirectUI Element Classes
| Class | Description |
|-------|-------------|
| `MiniMonthElement` | Compact month calendar display (has `ViewChange` event) |
| `Selector` (imported from UXCore) | Selection management |

## Interfaces
| Interface | Description |
|-----------|-------------|
| `IUXCalStringLoader` | Custom string loader for calendar localization |
| `IUXCalendarPopupCallback` | Callback when date picker completes |
| `ICalFreeBusyCheckProvider` | Free/busy time checking for calendar events |

## Imports from UXCore.dll
- `CRMDUIParser` — constructor, destructor, `Create@CRMDUIParser`, `CreateStringValue`
- `Element` — full lifecycle: `Initialize`, `Destroy`, `Add`, `Remove`, `Insert`, `DestroyAll`, `FindDescendent`, `FindDescendentByClass`
- Element properties: `Value@DirectUI` factories, `GetValue`, `SetValue`
- `FireEvent`, `OnInput`, `OnEvent`, `OnHosted`, `OnUnHosted`, `OnPropertyChanged`, `OnPropertyChanging`
- `Paint`, `PaintText`, `GetContentSize`, `GetTextLineMetrics`
- `SetDataContext`, `GetDataContext`, `SetDataTransform`, `EvaluateSubtreeBindings`
- `SetKeyFocus`, `EnsureVisible`, `DefaultAction`, `GetAccessibleImpl`
- `StrToID`, `FindFirstDescendentByClass`
- `PopupMenuHWNDHost::Create` — for hosting calendar popup
- Properties: `ChildrenProp`, `ContentProp`, `VisibleProp`, `SelectedProp`, `HeightProp`, `DesiredSizeProp`, `TagProp`, `IDProp`, `ParentProp`, `KeyWithinProp`, `AccStateProp`, `AccNameProp`, `CmdContextProp`, `CmdIDProp`, `KeyboardNavigate`
- `Button::Click` event, `WLEditT::Class`, `HWNDElement::Class`, `Button::Class`, `Selector::Class`
- `CreateInt`, `CreateBool`, `CreateString`, `CreateDouble`, `CreateUnknown`

## Imports from WLDCore.dll
- Ordinals 107, 102, 104 — likely locale/timezone services

## Imports from Other DLLs
| DLL | Purpose |
|-----|---------|
| MSVCR110.dll | C runtime (date/time formatting: `qsort`, `wcschr`, `wcsstr`, `iswdigit`, `iswspace`) |
| KERNEL32.dll | `GetCalendarInfoW`, `GetDateFormatW`, `GetTimeFormatW`, `GetLocaleInfoW`, `TzSpecificLocalTimeToSystemTime`, `SystemTimeToTzSpecificLocalTime`, `GetLocalTime` — heavy calendar/timezone API usage |
| USER32.dll | Menu functions (`CreatePopupMenu`, `InsertMenuItemW`, etc.) |
| ADVAPI32.dll | Registry (for calendar preferences), WMI tracing |
| OLEAUT32.dll | VARIANT handling for date values |
| ole32.dll | `StringFromGUID2`, `CoCreateInstance` |

## Key Architecture Notes
1. **Specialized calendar/date library** — focused on date parsing, formatting, and UI
2. **Locale-aware** — uses `GetCalendarInfoW`, `GetDateFormatW`, `GetTimeFormatW` for proper localization
3. **Non-Gregorian calendar support** — `DateUtils_RemapLocaleToGregorian` handles calendar system conversion
4. **ISO 8601 date format** — all internal dates use ISO string format
5. **Variant time (double)** — standard OLE date representation used for interop
6. **DirectUI integration** — `MiniMonthElement` provides calendar grid, `ShowPopUpDatePicker` provides popup calendar
7. **Free/Busy support** — `ICalFreeBusyCheckProvider` interface enables meeting scheduling UI
8. **Custom string loading** — `IUXCalStringLoader` allows apps to override calendar string localization
9. **Used by uxcontacts.dll** — contacts UI calls `FormatDateLocalized` and calendar init functions
10. **Timezone handling** — bidirectional conversion between local time and timezone-specific time
11. **User input parsing** — `UXParseUserDateString` and `UXParseUserTimeString` handle free-form date/time entry
