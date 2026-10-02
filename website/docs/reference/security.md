---
sidebar_position: 4
title: Security Policy
description: Untrusted-input bounds, credential handling, and the inert-telemetry rule — the non-negotiables.
---

# Security Policy

These rules are **non-negotiable** regardless of parity pressure. They come from
`AGENTS.md` and `StubDesign.md` — if a faithful reconstruction of some behavior would
violate them, the safe version wins and the divergence is documented.

## Untrusted-input bounds

Loaders that ingest files from disk must **bound their resource consumption**. The four
precedent implementations (extend these patterns — don't invent new ones):

| Module | Bounds |
|---|---|
| **WLXMP4Parser** | Bounds-checked box walker; `MoovBox` caps traks at **1024**; `ParseStts` accumulates sample counts in 64-bit before clamping |
| **HMREngine::X3DReader** | Nesting depth capped at **512**; node count capped at **100,000**; recursive JSON parsing depth-guarded |
| **HMREngine scene traversal** | `Traverse` / `TraverseNode` / `EnumerateRecursive` depth-capped at **512** (stack-exhaustion defense) |
| **WLXFaceRecognition** | Analysis downscaled to a **640 px** max dimension; bounded sample counts |

## Credential handling

**Never store plaintext credentials.**

- `AuthProvider` (`HMRAVSource/AuthProvider.cpp`) — in-memory cache only, cleared on
  shutdown; nothing plaintext touches disk.
- `PSAAuthenticationStore` (`External/PSAStub.cpp`) — persists to
  `%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat` as a single **DPAPI**
  (`CryptProtectData`, current-user scope) blob. The file is:
  - magic/version-header-gated,
  - size-bounded (**64 MB** cap),
  - length-prefixed fields with per-field bounds checks,
  - token-count capped (**4096 entries**).
- `crypt32` is linked by MovieMakerCore specifically for this.

## Telemetry must be inert

`WLXPhotoSqm` and `DmxBici` export their full original surfaces but are **inert: they
never send data off-machine**. No sockets, no telemetry files, no phoning home — ever.
Inert return behavior follows each export's signature rather than a blanket `S_OK`:
`HRESULT` exports return `S_OK`; `BOOL` exports return `TRUE` (e.g.
`BiciWrapper_TransferExperienceToWeb`); `DWORD`/`BOOL` queries return zero/`FALSE`
(e.g. `Sqm_GetOptInState`, `Sqm_IsEnabled`); `void` exports are no-ops.

## Secrets hygiene

- Never print, log, or commit secrets/tokens — including in test output and contracts.
- The contract suite must not embed real credentials; inert/empty tokens only.

## When in doubt

1. Check the [stub categories](../methodology/stub-design.md#stub-categories) —
   "Auth/storage" and "Telemetry/SQM" have explicit rules.
2. Prefer failing closed (`E_FAIL`) over silently succeeding on ambiguous input.
3. Add a contract for any security-relevant behavior you fix, so it can't regress.
