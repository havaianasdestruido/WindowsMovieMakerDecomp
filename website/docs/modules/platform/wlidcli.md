---
sidebar_position: 5
title: wlidcli.dll (Identity)
description: The Windows Live ID client surface — login, tickets, and environment queries with sparse historical ordinals.
---

# wlidcli.dll

| | |
|---|---|
| **Source** | `src/wlidcli/` |
| **CMake target** | `wlidcli` |
| **Type** | Win32 DLL |
| **Family** | [Platform services targets](/modules/platform) |

## At a glance

The **Windows Live ID client** surface: sign-in state, credential checks, tickets, and
identity handles. On modern Windows the service behind it no longer exists, so this is a
**delay-loaded optional** component — the app must survive its absence
([Quirk #11](../../methodology/quirks.md#11-delay-loaded-dlls-that-no-longer-exist)).

## Public surface

From `wlidcli.def` — note the **sparse, non-sequential ordinals**, inherited from the
original DLL's larger historical export table:

```text
WLClogin                @2
WLCheckCredentials      @3
WLCreateIdentityHandle  @8
WLGetTicket             @29
WLIsSignedIn            @41
WLFreeMemory            @108
WLGetEnvironment        @113
```

## Implementation notes

- The export ordinal gaps (2, 3, 8, 29, 41, 108, 113) are **part of the parity
  contract** — legacy consumers may bind by ordinal.
- Reconstruction behavior: the identity surface is **inert but secure** — no network
  calls, no credential persistence (the [auth/storage stub category](../../methodology/stub-design.md#stub-categories));
  tokens handed out are empty and `WLIsSignedIn` reports signed-out unless an in-memory
  session exists.
- Links only the base system set (`kernel32`, `user32`, `ole32`, `oleaut32`,
  `advapi32`).

## Testing

- Contract coverage (ordinals + inert behavior) in `tests/mmr-python`.

## Analysis artifacts

`analysis/Shared/` (wlidcli shares the WLX foundation analysis) and
`analysis/CrossDllImports/`.
