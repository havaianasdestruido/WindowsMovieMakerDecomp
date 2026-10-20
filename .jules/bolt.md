## 2025-05-15 - Unnecessary std::string copies during parsing
**Learning:** The `X3DReader` parser creates a full `std::string` copy of the entire file buffer (which could be quite large) just for basic parsing and dispatching (XML vs JSON vs X3D).
**Action:** Replace `std::string(data, length)` with an in-place read using `std::streambuf` for text-line processing, and use memory functions (`std::memchr`) for basic XML/JSON parsing to avoid allocating a huge string just to read from memory.

## 2023-11-20 - Unnecessary std::string copies during ParseX3D
**Learning:** Checking for format strings in large file buffers can be done efficiently using zero-copy standard C memory functions (`std::memcmp`) instead of creating large `std::string` copies to use `.find()`.
**Action:** Replace `std::string(data, ...)` format checking with custom inline `contains` logic traversing `data` via `std::memcmp`, especially for file readers that can have large byte arrays, avoiding allocations.

## 2023-10-25 - X3D Array Parsing Allocations
**Learning:** Parsing massive float/int arrays in `X3DReader` using `Split` created millions of temporary `std::string` objects and threw expensive exceptions for commas via `std::stof`.
**Action:** Replace `Split` and string manipulation with direct in-place `const char*` pointer traversal and `std::strtof`/`std::strtol` for zero-allocation parsing of 3D data arrays.
## 2024-05-18 - Mesh Resource Generation Bottleneck
**Learning:** Generating mesh resources (like grids and wipe meshes) dynamically can cause unnecessary memory reallocations if the target vertex and index buffers are not pre-reserved. In `MeshResourceDX.cpp`, vectors for vertices and indices were populated via `push_back` inside nested loops without prior capacity reservations, causing O(N) allocation overhead for meshes with many subdivisions.
**Action:** Always compute and `reserve()` the exact or maximum required size for `std::vector` before populating it in tight loops, particularly for procedural geometry generation (e.g., `(segmentsX + 1) * (segmentsY + 1)` for a grid).
## 2024-12-05 - TextImpl Rebuild Pattern
**Learning:** `TextImpl` bakes spatial position directly into vertices (e.g. `Vec3 p0(m_position.x + x, m_position.y - y, m_position.z);`) instead of using a constant buffer/transform matrix. This architectural choice means any positional update necessitates a full recreation of the vertex buffer.
**Action:** When optimizing components like `TextImpl`, recognize that standard optimizations like avoiding buffer recreations might require significant architectural changes. For now, focus on optimizing the rebuild process itself (e.g., via `reserve`).
