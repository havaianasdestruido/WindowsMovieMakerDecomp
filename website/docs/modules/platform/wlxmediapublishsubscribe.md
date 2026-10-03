---
sidebar_position: 2
title: WLXMediaPublishSubscribe.dll
description: The publish/subscribe framework — 21 flat PublishManager_* exports plus COM registration.
---

# WLXMediaPublishSubscribe.dll

| | |
|---|---|
| **Source** | `src/WLXMediaPublishSubscribe/` |
| **CMake target** | `WLXMediaPublishSubscribe` |
| **Type** | Win32 COM DLL |
| **Family** | [Platform services targets](/modules/platform) |

## At a glance

The **publish/subscribe framework** behind "Save movie → online service": enumerates
publish targets (local file, DVD, online services), manages authentication and account
info, and runs publish/subscribe jobs with progress and completion callbacks.

## Public surface

The COM quartet plus **21 flat `PublishManager_*` exports**
(`WLXMediaPublishSubscribe.def`):

| Group | Exports |
|---|---|
| Lifecycle | `Create`, `Destroy`, `Cleanup` |
| Targets | `EnumerateTargets`, `GetDefaultTarget`, `SetDefaultTarget`, `GetTargetName` |
| Auth | `Authenticate`, `IsAuthenticated`, `SignOut`, `RefreshToken`, `GetAccountInfo` |
| Status | `GetStatus`, `GetServiceStatus`, `GetSubscribeStatus`, `GetResult` |
| Jobs | `StartPublish`, `StartSubscribe`, `Cancel` |
| Callbacks | `SetProgressCallback`, `SetCompleteCallback` |

All `__stdcall` (decorated `@N`).

## Authentication model

`Authenticate`/`RefreshToken` route through the identity layer; persistent tokens are
**never plaintext** — see [Credential handling](../../reference/security.md#credential-handling).
The original service plugins (Facebook, Flickr, Vimeo, YouTube) were separate DLLs; their
binary analysis lives in `analysis/PublishPlugins/`.

## Implementation notes

- Publish jobs report progress via the callback pair; `Cancel` is cooperative.
- Contract-pinned factory + selected flat exports in `tests/mmr-python`.
- Exercise app: `apps/mediapublisher` (submodule).

## Analysis artifacts

`analysis/WLXMediaPublishSubscribe/`, `analysis/PublishPlugins/`,
`analysis/PublishPluginsInterop/`, `analysis/WLFacebookPlugin/`, `analysis/WLFlickrPlugin/`,
`analysis/WLVimeoPlugin/`.
