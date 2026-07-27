# D3DCOMPILER_46.dll — Analysis

## Identity

| Property | Value |
|----------|-------|
| File | `undecomp\Photo Gallery\D3DCOMPILER_46.dll` |
| Purpose | DirectX 11 HLSL Shader Compiler (D3DX11 Effect Framework) |
| Machine | x86 (PE32) |
| Timestamp | Tue Jul 10 05:29:14 2012 |
| Linker | 10.10 |
| OS Version | 6.02 (Windows 8) |
| PDB | `D3DCompiler_46.pdb` — GUID `{D5FB6020-1B02-4F2B-97FB-AC826EE690CC}` |
| Image Base | `0x10000000` |
| Size of Image | `0x319000` (~3.1 MB) |
| Subsystem | Windows CUI (3) |
| DLL Characteristics | Dynamic base, NX compatible |
| Sections | `.text` (2.9 MB), `.data` (43 KB), `.idata` (4.6 KB), `.rsrc` (1 KB), `.reloc` (113 KB) |

## Exported APIs (25 functions)

### Core Shader Compilation

| Ordinal | Hint | Name | Purpose |
|---------|------|------|---------|
| 4 | 1 | `D3DCompile` | Compile HLSL source to bytecode (primary API) |
| 3 | 2 | `D3DCompile2` | Compile with extended parameters (effects, etc.) |
| 5 | 3 | `D3DCompileFromFile` | Compile from file path string |
| 19 | 11 | `D3DPreprocess` | HLSL preprocessor only (no codegen) |

### Shader Assembly / Disassembly

| Ordinal | Hint | Name | Purpose |
|---------|------|------|---------|
| 1 | 0 | `D3DAssemble` | Assemble HLSL assembly text to bytecode |
| 11 | 7 | `D3DDisassemble` | Disassemble bytecode to readable text |
| 9 | 8 | `D3DDisassemble10Effect` | Disassemble D3D10 effect bytecode |
| 10 | 9 | `D3DDisassemble11Trace` | Disassemble D3D11 shader trace |
| 12 | 10 | `D3DDisassembleRegion` | Disassemble a region of bytecode |

### Shader Compression / Decompression

| Ordinal | Hint | Name | Purpose |
|---------|------|------|---------|
| 6 | 4 | `D3DCompressShaders` | Compress multiple shader bytecode blobs |
| 8 | 6 | `D3DDecompressShaders` | Decompress compressed shader blob |

### Blob / Signature Operations

| Ordinal | Hint | Name | Purpose |
|---------|------|------|---------|
| 7 | 5 | `D3DCreateBlob` | Create a generic data blob |
| 13 | 11 | `D3DGetBlobPart` | Extract a part from a bytecode blob |
| 23 | 15 | `D3DSetBlobPart` | Replace a part in a bytecode blob |
| 20 | 12 | `D3DReadFileToBlob` | Read a file into a blob |
| 25 | 17 | `D3DWriteBlobToFile` | Write a blob to a file |

### Shader Signature / Reflection

| Ordinal | Hint | Name | Purpose |
|---------|------|------|---------|
| 15 | 13 | `D3DGetInputAndOutputSignatureBlob` | Get both I/O signatures |
| 16 | 14 | `D3DGetInputSignatureBlob` | Get input signature from bytecode |
| 17 | 15 | `D3DGetOutputSignatureBlob` | Get output signature from bytecode |
| 21 | 13 | `D3DReflect` | Create ID3D11ShaderReflection interface |

### Shader Stripping / Utilities

| Ordinal | Hint | Name | Purpose |
|---------|------|------|---------|
| 24 | 16 | `D3DStripShader` | Strip debug/signature info from bytecode |
| 18 | 10 | `D3DGetTraceInstructionOffsets` | Get instruction offsets for traces |
| 14 | 12 | `D3DGetDebugInfo` | Get debug info from bytecode |
| 22 | 14 | `D3DReturnFailure1` | Internal failure handler |
| 2 | 18 | `DebugSetMute` | Toggle debug output muting |

## DLL Imports (4 system DLLs)

| System DLL | Functions | Purpose |
|------------|-----------|---------|
| **KERNEL32.dll** | 72 | File I/O, memory, threading, TLS, exception handling, LoadLibrary |
| **ADVAPI32.dll** | 10 | Registry access (open/query/enum keys), crypto hashing (CryptHashData) |
| **RPCRT4.dll** | 1 | UuidCreate — generate unique shader/effect GUIDs |
| **msvcrt.dll** | 103 | C runtime: malloc/free, string ops, math (_CIcos/_CIsin/etc.), C++ exception handlers, qsort, sscanf |

### Notable ADVAPI32 Usage
The crypto imports (`CryptCreateHash`, `CryptHashData`, `CryptGetHashParam`) indicate the compiler hashes HLSL source for cache/fingerprint purposes, likely for incremental compilation or shader cache validation.

### Notable msvcrt Math Imports
The `_CIacos`, `_CIasin`, `_CIatan`, `_CIcos`, `_CIsin`, `_CIsqrt`, `_CIpow` etc. are compiler-generated intrinsics for HLSL intrinsic functions — the shader compiler must evaluate constant expressions at compile time using these.

## Cross-DLL Import Map

**Only one Movie Maker DLL imports D3DCOMPILER_46.dll:**

| Consumer DLL | Functions Imported | Purpose |
|--------------|-------------------|---------|
| **MovieMakerCore.dll** | 2 | `D3DGetInputSignatureBlob`, `D3DReflect` |

### Usage Context in MovieMakerCore.dll

MovieMakerCore.dll uses D3DCOMPILER_46.dll for **shader introspection only** — not compilation. The two imported APIs serve:

1. **`D3DGetInputSignatureBlob`** — Extract the input signature from pre-compiled shader bytecode, used to match vertex buffer layouts to shader inputs.
2. **`D3DReflect`** — Create an `ID3D11ShaderReflection` interface from bytecode, used to query shader constants, textures, samplers, and buffer bindings for D3D11 rendering setup.

This means MovieMakerCore.dll does **not** compile HLSL at runtime. It works with pre-compiled shader bytecode (embedded as resources or binary blobs) and uses D3DCOMPILER_46.dll's reflection layer to set up the rendering pipeline.

## Role in Windows Movie Maker Architecture

```
MovieMakerCore.dll
  └── D3DCOMPILER_46.dll (2 fns)
        ├── D3DGetInputSignatureBlob — shader input layout matching
        └── D3DReflect               — shader resource/buffer introspection
              └── Used for D3D11 GPU-accelerated rendering
                   (transitions, effects, video preview compositing)
```

The D3D11 rendering pipeline in MovieMakerCore handles GPU-accelerated video transitions, effects, and compositing. The shader compiler DLL provides the reflection layer needed to bind GPU resources correctly without recompiling shaders at runtime.
