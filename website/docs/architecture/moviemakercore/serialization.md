---
sidebar_position: 8
title: Serialization (.wlmp)
description: Project file I/O — the XmlLite reader/writer, serialization classes, and bound placeholders.
---

# Serialization

**Location:** `src/MovieMakerCore/StoryboardManager/Serialization/`

Movie Maker projects are `.wlmp` files — XML written with **XmlLite**
(`XmlLite.lib`), parsed with an `IXmlReader`.

## Components

| Component | Role |
|---|---|
| `SerializationContext` | Shared reader/writer state: version negotiation, error reporting, ID↔object maps |
| `SerializationReader` | `IXmlReader`-driven XML → model pass |
| `SerializationWriter` | model → XML pass (`IXmlWriter`) |
| `SerializationClasses` | per-element (de)serialization: project, tracks, extents, effects, themes |
| `BoundPlaceholder` | placeholder objects that resolve late (theme slots) |

## Format handling

- Reader supports the project versions the original did, including the **legacy v1
  format** — dead code in practice, shipped-but-unused
  ([Quirk #19](../../methodology/quirks.md#19-legacy-project-format-support)).
- Round-trip fidelity is contract-tested: writing then reading a project preserves model
  state (some **round-trip edge cases remain known stubs** — tracked by the
  [TODO audit](../../testing/todo-audit.md)).

## XmlLite compatibility notes

Two Win10-SDK API differences were adapted during reconstruction (see
[SDK Compatibility](../../reference/sdk-compatibility.md)):

- `IXmlReader::GetAttribute` does not exist → replaced with
  `MoveToAttributeByName` + `GetValue`.
- `XmlWriterProperty_ProcessNamespaces` was removed from the SDK → defined locally as
  `((XmlWriterProperty)1)`.
- `WriteEndElement()` / `WriteEndDocument()` take 0 arguments (the original code
  base assumed defaults — Gotchas #16 in `ROADMAP.md`).

## Known preserved defect

The serialization writer contains a **memory leak on the error path** — faithfully
preserved from the original binary
([Quirk #23](../../methodology/quirks.md#23-memory-leak-in-serialization-writer)). Do not
"fix" it without an explicit remix request; the contract suite documents the behavior.
