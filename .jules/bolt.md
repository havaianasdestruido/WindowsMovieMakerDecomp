## 2025-05-15 - Unnecessary std::string copies during parsing
**Learning:** The `X3DReader` parser creates a full `std::string` copy of the entire file buffer (which could be quite large) just for basic parsing and dispatching (XML vs JSON vs X3D).
**Action:** Replace `std::string(data, length)` with an in-place read using `std::streambuf` for text-line processing, and use memory functions (`std::memchr`) for basic XML/JSON parsing to avoid allocating a huge string just to read from memory.

## 2023-11-20 - Unnecessary std::string copies during ParseX3D
**Learning:** Checking for format strings in large file buffers can be done efficiently using zero-copy standard C memory functions (`std::memcmp`) instead of creating large `std::string` copies to use `.find()`.
**Action:** Replace `std::string(data, ...)` format checking with custom inline `contains` logic traversing `data` via `std::memcmp`, especially for file readers that can have large byte arrays, avoiding allocations.

## 2023-10-25 - X3D Array Parsing Allocations
**Learning:** Parsing massive float/int arrays in `X3DReader` using `Split` created millions of temporary `std::string` objects and threw expensive exceptions for commas via `std::stof`.
**Action:** Replace `Split` and string manipulation with direct in-place `const char*` pointer traversal and `std::strtof`/`std::strtol` for zero-allocation parsing of 3D data arrays.
