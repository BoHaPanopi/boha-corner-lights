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
// Diagnostic probe: records distinct unknown (marker bits, mask) flare pairs so
// real in-game values (e.g. stock fog/aux flares) can be collected on the pinned
// build without guessing offsets. Purely observational; never drives a light.
inline constexpr unsigned ProbeCapacity = 32;
inline std::atomic<unsigned long long> probeTable[ProbeCapacity];
inline std::atomic<unsigned> pairClaims[ProbeCapacity]; // instance slots used per pair
// Finds or claims the slot index of a pair key; fresh reports a new claim.
inline int PairSlot(unsigned long long key, bool& fresh) {
    fresh = false;
    if (!key) return -1; // zero key is the empty-slot sentinel
    for (unsigned i = 0; i < ProbeCapacity; ++i) {
        unsigned long long current = probeTable[i].load(std::memory_order_relaxed);
        if (current == key) return static_cast<int>(i);
        if (current == 0) {
            unsigned long long expected = 0;
            if (probeTable[i].compare_exchange_strong(expected, key)) {
                fresh = true;
                return static_cast<int>(i);
            }
            if (expected == key) return static_cast<int>(i);
        }
    }
    return -1; // table full: stay silent instead of spamming the log
}
inline bool RecordUnknownSource(float marker, unsigned mask) {
    bool fresh = false;
    PairSlot((static_cast<unsigned long long>(Bits(marker)) << 32) | mask, fresh);
    return fresh;
}
// Probe v2.2: bounded lock-free per-instance tracking of the native gate result.
// Each physical light is a distinct native instance, so transitions of the
// original return value let one session map light circuits by timestamps.
// Slots are claimed only on the first observed activation (measured spawn
// baseline of 19.11.2026: flare instances start unlit) AND only within a short
// window after a player light-channel change reported by telemetry, so lit AI
// traffic no longer floods the table. A per-pair budget additionally prevents a
// single light type from consuming the capacity. Rejections are counted at most
// once per second so the counter approximates rejection time, not call volume.
// Tracked instances keep reporting transitions regardless of the window.
// Purely observational; never drives a light and uses no new offsets.
inline constexpr unsigned ProbeInstanceCapacity = 128;
inline constexpr unsigned ProbeTransitionLimit = 16;
inline constexpr unsigned ProbePairBudget = 16;
inline constexpr unsigned long long ProbeWindowMs = 1500;
struct ProbeInstance {
    std::atomic<const void*> instance{nullptr};
    std::atomic<unsigned long long> key{0}; // marker bits << 32 | mask
    std::atomic<unsigned> state{0};         // (reported count << 1) | last lit bit
};
inline ProbeInstance probeInstances[ProbeInstanceCapacity];
// Tick of the last player light-channel change; 0 keeps claims disabled, which
// is the safe passive fallback when no light channel could be registered.
inline std::atomic<unsigned long long> lastPlayerLightChange{0};
inline bool InProbeWindow(unsigned long long now) {
    const auto last = lastPlayerLightChange.load(std::memory_order_relaxed);
    return last != 0 && now >= last && now - last <= ProbeWindowMs;
}
inline std::atomic<unsigned> probeRejected{0}; // seconds containing rejected claims
inline std::atomic<unsigned long long> rejectedSecond{~0ull};
inline void CountRejected(unsigned long long now) {
    const unsigned long long second = now / 1000;
    auto last = rejectedSecond.load(std::memory_order_relaxed);
    if (last != second && rejectedSecond.compare_exchange_strong(last, second))
        probeRejected.fetch_add(1, std::memory_order_relaxed);
}
// Returns the 1-based report number when the observation should be logged,
// 0 when it is untracked-and-unclaimable, a repeat or over budget.
inline unsigned RecordSourceState(const void* instance, float marker, unsigned mask,
                                  bool lit, unsigned long long now) {
    if (!instance) return 0;
    const unsigned long long key =
        (static_cast<unsigned long long>(Bits(marker)) << 32) | mask;
    for (auto& slot : probeInstances) {
        const void* current = slot.instance.load(std::memory_order_acquire);
        if (current == nullptr) {
            // Claims fill slots front to back, so an empty slot means the
            // instance is untracked. Dark sources are not worth a slot, and
            // claims happen only inside a player light-change window.
            if (!lit || !InProbeWindow(now)) return 0;
            bool fresh = false;
            const int pair = PairSlot(key, fresh);
            if (pair < 0) { CountRejected(now); return 0; } // pair table full
            unsigned claims = pairClaims[pair].load(std::memory_order_relaxed);
            for (;;) {
                if (claims >= ProbePairBudget) { CountRejected(now); return 0; }
                if (pairClaims[pair].compare_exchange_weak(claims, claims + 1)) break;
            }
            const void* expected = nullptr;
            if (slot.instance.compare_exchange_strong(expected, instance)) {
                slot.key.store(key, std::memory_order_relaxed);
                slot.state.store((1u << 1) | 1u, std::memory_order_release);
                return 1; // first observed activation
            }
            pairClaims[pair].fetch_sub(1, std::memory_order_relaxed); // slot lost to a racer
            if (expected != instance) continue;
        } else if (current != instance) {
            continue;
        }
        unsigned state = slot.state.load(std::memory_order_acquire);
        for (;;) {
            const unsigned count = state >> 1;
            if (count == 0) return 0;                    // claim still in flight elsewhere
            if (((state & 1u) != 0) == lit) return 0;    // no transition
            if (count >= ProbeTransitionLimit) return 0; // per-instance budget used up
            const unsigned next = ((count + 1u) << 1) | (lit ? 1u : 0u);
            if (slot.state.compare_exchange_weak(state, next)) return count + 1u;
        }
    }
    if (lit && InProbeWindow(now)) CountRejected(now);
    return 0; // table full
}
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
