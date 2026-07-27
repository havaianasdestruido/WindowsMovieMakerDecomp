# UXContacts.dll — Windows Live Contacts UI

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Timestamp | Tue Apr 1 01:24:19 2014 (0x533A3F73) |
| Image Base | 0x10000000 |
| Size of Image | 0x93000 (~588 KB) |
| Code Size | 0x72E00 (~462 KB) |
| Subsystem | Windows GUI (2) |
| PDB | `uxcontacts.pdb` — GUID `{9BE2931C-A2A1-4230-8FA5-73FC1C201B99}` |
| C/C++ Functions | 108 |

## Exports Summary
- **Total exports:** 733 functions (ordinal base 10)
- **DLL entry points:** `DllCanUnloadNow`, `DllGetClassObject`, `DllMain`, `DllRegisterServer`, `DllUnregisterServer`
- **Core API:** `InitializeUXContacts`, `TerminateUXContacts`, `UXContactsCreateObject`, `UXContactsInitProcess`, `UXContactsUnInitProcess`

## DirectUI Element Classes
This DLL registers its own DirectUI element classes for contact UI:

| Class | Description |
|-------|-------------|
| `CAddressWellElement` | "To:" address well (email recipient input) |
| `CBuddyListElement` | Buddy/contact list element |
| `CBuddyListElementBase` | Base buddy list |
| `CBuddyVirtualList` | Virtualized buddy list |
| `CContactElemBase` | Base contact element |
| `CContactElement` | Single contact display |
| `CContactRegionElement` | Contact region (group header) |
| `CListItemElement` | Generic list item |
| `CMobileAddressSelector` | Mobile address selector |
| `CMobileSelector` | Mobile number selector |
| `CPPListElement` | People Picker list element |
| `CPeoplePickerElem` | People Picker control |
| `CWordWheelElement` | Search/auto-complete "word wheel" |

## Core Contacts API (`Contacts@@` namespace)
Static methods on the `Contacts` class:

| Method | Description |
|--------|-------------|
| `AddContactToMyProfile` | Add contact to user's own profile |
| `AddToFavorites` (2 overloads) | Add contact(s) to favorites |
| `CanMobileMessage` (2 overloads) | Check if contact supports mobile messaging |
| `GetMyProfile` / `ShowMyProfile` | View own profile |
| `ShowProfile` (2 overloads) | View someone else's profile (by CID or ISimpleContact) |
| `SetDefaultTransform` | Set default UI transform for contact display |
| `SetHost` | Set buddy list host |
| `SetMarket` | Set market/locale |
| `SetNameFormat` | Set name display format |
| `SetNeedToImportOutlookContacts` | Enable/disable Outlook import |
| `SetNeedToImportWABContacts` | Enable/disable WAB import |
| `Synchronize` | Trigger contact sync |
| `UpdatePolicy` | Update contact policy |
| `ViewMap` | Map address type for display |
| `WaitForPendingChanges` | Wait for pending sync changes |

### Name Display Format
`SetNameFormat@Contacts@@YGJW4NameDisplayFormat@1@@Z` — enum `NameDisplayFormat` controls how contact names are displayed.

## Dialog Classes

### CContactPickerDialog
Full contact picker dialog with extensive API:
- `ShowModal` — Show modal picker, returns participant list
- `ShowModeless` — Show modeless picker with listener callback
- `SetCapability` — Set picker capabilities (enum `CAPABILITY`)
- `SetContactPickerListener` — Set completion callback
- `SetDataSource` — Set AB (Address Book) data source
- `SetDialogLabel` / `SetDialogLabelByID` / `SetDialogWindowTitleByID`
- `SetExpansion` — Set expansion mode
- `SetFilter` — Set filter type
- `SetPopulate` — Enable population
- `SetRestrictedUsers` — Restrict selectable users
- `SetSelectionLimit` — Max selections
- `SetStyle` — Contact picker style (enum `ContactPickerStyle`)
- `SetVisibleElements` — Control which UI elements are shown

### CContactDialog
Contact detail dialog:
- `ShowModal`, `ShowModeless`
- `SetISimpleContact`, `SetISimpleStringProperty`
- `SetParentHWND`
- `ShowOrHideMobilePhoneHelpString` — market-specific UI

### CAddContactDialog
Add new contact dialog:
- `ShowModal`, `ShowModeless`
- `SetISimpleContact`, `SetISimpleStringProperty`
- `SetDataStore`, `SetParentHWND`
- `SetProviderId`

### CABDialog / ABDialogPeoplePickerCommon
Address Book dialog base:
- `SetFocusOnElement`
- `SetPeoplePickerSelection`

## Data Source Interfaces
- `IABDataSource` — Address Book data source interface
- `IStoreObject` / `IStoreObjectView` — Data store objects
- `ISimpleContact` — Contact data interface
- `IParticipantList` — Participant list
- `IParticipantSelectionControl` — Selection change callback
- `IContactPickerListener` — Contact picker completion callback
- `IBuddyListHost` — Buddy list host interface
- `IABViewFilter` — View filter
- `IResultCallback` — Async result callback

## Filter System
| Class | Description |
|-------|-------------|
| `CViewFilter` | Base view filter |
| `CPeoplePickerViewFilter` | People picker filter |
| `CBuddyIdentifierFilter` | Filter by buddy identifier |
| `CCircleFilter` | Filter by social circle |
| `CEmailFilter` | Filter by email |
| `CGroupFilter` | Filter by group |
| `CPassportFilter` | Filter by Passport/Windows Live ID |

## CABPopupMenu
Context menu for contacts:
- `ShowPopup` — Show at point relative to element
- `SetDuiElement` — Set associated DirectUI element
- `SetReferencedEmail` — Set reference email
- `SetReferencedObject` — Set reference store object
- `SetReferencedParticipantList` — Set reference participant list
- `RemoveContactFromGroup` — Remove contact from group
- `ChangeEmailMenuItemText` — Dynamic menu item text
- `UpdateContactWithGroupChange` — Update contact on group change

## Data Objects
- `CABDataObject` — OLE data object for contacts (drag-drop, clipboard)
  - `SetData`, `GetData`, `SetStoreObjects`
  - `CLSID_WELLOLEOBJECT` — OLE class ID for well objects

## Grouping
- `CBaseGrouping` — Base grouping class
- `CWordWheelGrouping` — Grouping by word wheel search
- `SetStoreHandle`, `AddToList`

## Address Well (Email Recipient Input)
`CAddressWellElement` — The "To:/Cc:/Bcc:" input field:
- `AddParticipantToWell` — Add participant
- `AffectParticipant` — Modify participant (add/remove)
- `CancelResolve` — Cancel name resolution
- `CheckNames` — Resolve typed names
- `SelectInsertPosition` — Cursor position for insertion
- `SetSelection` / `SetSelectionChangeCallback`
- `SetText` — Set address text
- `SetExpandOn` — Set expansion behavior
- `SetDataStoreHandle`
- `TemplateProp` — Template property
- `SeparatorProp` — Separator character

## CtryCode — Country Code Utilities
- `CtryCode` class — Country code lookup and formatting
- `findCountryfromNumeric`, `findCtry` (2 overloads) — Lookup by numeric/name
- `SaveCountryToRegistry`, `GetDefaultCtryOpt`, `RemoveDefaultCtryOpt`
- `SelectCurrentCountryInComboBox`

## CPhoneRegUtil — Phone Registration
- `sm_strCountryCode` — Static country code string
- Utility functions for phone number formatting

## Participant Events
- `ParticipantEvent` — Events from participant interactions
- `ListNotificationEvent` — List change notifications
  - `SelectionChange` event pointer

## Key Properties (PropertyInfo)
| Class | Property | Description |
|-------|----------|-------------|
| `CAddressWellElement` | `SeparatorProp` | Separator between addresses |
| `CAddressWellElement` | `TemplateProp` | Address template |
| `CBuddyListElementBase` | `SelectFirstItemProp` | Auto-select first item |
| `CContactElemBase` | `ViewsProp` | View configuration |
| `CPeoplePickerElem` | `SelectionLimitProp` | Max selection count |
| `CPeoplePickerElem` | `VisibleElementsProp` | Visible UI elements |
| `CPPListElement` | `SelectionLimitProp` | Max selection count |
| `CPPListElement` | `RemoveScrollbarProp` | Hide scrollbar |

## Imports from Other DLLs
| DLL | Purpose |
|-----|---------|
| UXCore.dll | Full DirectUI framework (Element, Value, Layout, Parser, etc.) |
| UXCalendar.dll | Calendar integration (`FormatDateLocalized`, `UXCalendarInitProcess`, `UXAttachCalStringLoader`) |
| uxctl.dll | Ordinal 10 — likely a control factory |
| WLDCore.dll | Windows Live core (ordinals 14, 16, 101, 100, 18, 17, 10) |
| SHELL32.dll | `SHGetKnownFolderPath` |
| MSVCR110.dll | C runtime |
| KERNEL32.dll | Win32 core |
| USER32.dll | Window management |
| ADVAPI32.dll | Registry, tracing |
| GDI32.dll | GDI drawing |
| ole32.dll | COM/OLE |
| OLEAUT32.dll | Automation |
| SHLWAPI.dll | Shell utility |

## Key Architecture Notes
1. **Self-contained contacts UI module** — 462 KB of code, 733 exports
2. **Uses DirectUI throughout** — all UI elements extend `Element@DirectUI`
3. **Rich contact model** — supports names, email, mobile, groups, favorites, profiles
4. **Multiple dialog types** — picker, detail, add-contact, address book
5. **Filter system** — flexible filtering by circle, email, group, passport, etc.
6. **OLE integration** — drag-drop support via `CABDataObject`
7. **Country code system** — `CtryCode` class handles international phone numbers
8. **Depends on UXCalendar** for date formatting in contact UIs
9. **Name display format** is configurable via `SetNameFormat`
10. **Market-aware** — UI adapts based on market/locale setting
