#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>
namespace corner {
struct VehicleProfile {
    const char* id;
    const char* modelAdapter;
    float leftMarker;
    float rightMarker;
    float steeringOnDegrees;
    float steeringOffDegrees;
    float steeringMaxSpeedKmh;
    float lightRangeMeters;
    float innerAngleDegrees;
    float outerAngleDegrees;
    float yawDegrees;
    float pitchDegrees;
};
inline unsigned Bits(float f) { unsigned i; std::memcpy(&i, &f, 4); return i; }
inline constexpr VehicleProfile Profiles[] = {
    {"volvo-9700-dd-enrique-front-slots",
     "vehicle/bus/upgrade/enrique_9700/ec_fpanel.pmg",
     0.127811f, 0.127812f, 15.0f, 10.0f, 40.0f, 25.0f, 45.0f, 110.0f, 50.0f, -7.0f}
};
inline const VehicleProfile* FindProfile(float marker, unsigned mask, int& side) {
    side = 0;
    if (mask != 0x2000) return nullptr;
    for (const auto& profile : Profiles) {
        if (Bits(marker) == Bits(profile.leftMarker)) {
            side = 1;
            return &profile;
        }
        if (Bits(marker) == Bits(profile.rightMarker)) {
            side = 2;
            return &profile;
        }
    }
    return nullptr;
}
inline std::atomic<bool> left{false}, right{false}, electric{false};
inline std::atomic<bool> active{false};
inline std::atomic<float> steering{std::numeric_limits<float>::quiet_NaN()};
inline std::atomic<float> speed{std::numeric_limits<float>::quiet_NaN()};
inline std::atomic<bool> steerLeft{false}, steerRight{false};
inline void UpdateSteering(float normalized, float speedMps) {
    // Positive SDK steering is counterclockwise (left). The current wheel range is 900 degrees.
    if (!std::isfinite(normalized) || !std::isfinite(speedMps) ||
        std::fabs(normalized) > 1.001f ||
        std::fabs(speedMps) >= Profiles[0].steeringMaxSpeedKmh / 3.6f) {
        steerLeft=false; steerRight=false; return;
    }
    const float degrees=normalized*450.0f;
    steerLeft = degrees >= (steerLeft.load() ? Profiles[0].steeringOffDegrees :
                                               Profiles[0].steeringOnDegrees);
    steerRight = degrees <= -(steerRight.load() ? Profiles[0].steeringOffDegrees :
                                                 Profiles[0].steeringOnDegrees);
}
inline std::atomic<unsigned long long> lastSample{0};
inline int Side(float f, unsigned mask) {
    int side;
    FindProfile(f, mask, side);
    return side;
}
inline bool Enabled(int side, unsigned long long now) {
    const auto last = lastSample.load();
    if (!last || now - last > 1500 || !electric.load() || !active.load()) return false;
    return side == 1 ? (left.load() || steerLeft.load()) :
           side == 2 ? (right.load() || steerRight.load()) : false;
}
}
