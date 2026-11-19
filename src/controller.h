#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>
namespace corner {
inline constexpr float LeftMarker = 0.127811f;
inline constexpr float RightMarker = 0.127812f;
inline std::atomic<bool> left{false}, right{false}, electric{false};
inline std::atomic<bool> active{false};
inline std::atomic<float> steering{std::numeric_limits<float>::quiet_NaN()};
inline std::atomic<float> speed{std::numeric_limits<float>::quiet_NaN()};
inline std::atomic<bool> steerLeft{false}, steerRight{false};
inline void UpdateSteering(float normalized, float speedMps) {
    // Positive SDK steering is counterclockwise (left). The current wheel range is 900 degrees.
    if (!std::isfinite(normalized) || !std::isfinite(speedMps) ||
        std::fabs(normalized) > 1.001f || std::fabs(speedMps) >= 40.0f / 3.6f) {
        steerLeft=false; steerRight=false; return;
    }
    const float degrees=normalized*450.0f;
    steerLeft = degrees >= (steerLeft.load() ? 10.0f : 15.0f);
    steerRight = degrees <= -(steerRight.load() ? 10.0f : 15.0f);
}
inline std::atomic<unsigned long long> lastSample{0};
inline unsigned Bits(float f) { unsigned i; std::memcpy(&i, &f, 4); return i; }
inline int Side(float f, unsigned mask) {
    if (mask != 0x2000) return 0;
    if (Bits(f) == Bits(LeftMarker)) return 1;
    if (Bits(f) == Bits(RightMarker)) return 2;
    return 0;
}
inline bool Enabled(int side, unsigned long long now) {
    const auto last = lastSample.load();
    if (!last || now - last > 1500 || !electric.load() || !active.load()) return false;
    return side == 1 ? (left.load() || steerLeft.load()) :
           side == 2 ? (right.load() || steerRight.load()) : false;
}
}
