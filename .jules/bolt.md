## 2025-05-15 - Unnecessary std::string copies during parsing
**Learning:** The `X3DReader` parser creates a full `std::string` copy of the entire file buffer (which could be quite large) just for basic parsing and dispatching (XML vs JSON vs X3D).
**Action:** Replace `std::string(data, length)` with an in-place read using `std::streambuf` for text-line processing, and use memory functions (`std::memchr`) for basic XML/JSON parsing to avoid allocating a huge string just to read from memory.
