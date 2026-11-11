## 2024-06-25 - X3DReader String Allocation Bottlenecks
**Learning:** In C++ codebases processing large text files line-by-line (like `X3DReader::ParseX3DV1`), using `std::getline` with an `std::istream` or copying sub-strings with `substr()` prior to trimming causes massive unnecessary heap allocation overhead.
**Action:** Replace `std::getline` and `substr()` with direct pointer traversal and bounds scanning (`std::memchr`, `find_first_not_of`) over a loaded memory buffer to completely eliminate intermediate `std::string` copies and stream-wrapper overhead.
