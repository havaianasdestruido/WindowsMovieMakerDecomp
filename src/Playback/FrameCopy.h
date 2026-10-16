#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace DirectUI {
// Validate the entire RGB32 frame before touching either buffer (#24).
inline bool CopyRGB32Rows(unsigned char* dst, size_t dstPitch,
                         const unsigned char* src, size_t cbData,
                         size_t width, size_t height, int32_t stride) {
    if (!dst || !src || !width || !height ||
        width > (std::numeric_limits<size_t>::max)() / 4)
        return false;
    const size_t rowBytes = width * 4;
    const uint64_t pitch64 = stride < 0 ? -int64_t(stride) : int64_t(stride);
    if (pitch64 > (std::numeric_limits<size_t>::max)()) return false;
    const size_t pitch = static_cast<size_t>(pitch64);
    if (pitch < rowBytes || dstPitch < rowBytes || cbData < rowBytes ||
        height - 1 > (cbData - rowBytes) / pitch ||
        height > (std::numeric_limits<size_t>::max)() / dstPitch)
        return false;
    for (size_t row = 0; row < height; ++row) {
        const size_t sourceRow = stride < 0 ? height - 1 - row : row;
        std::memcpy(dst + row * dstPitch, src + sourceRow * pitch, rowBytes);
    }
    return true;
}
} // namespace DirectUI
