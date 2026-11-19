#pragma once
#include <cstddef>
#include <cstring>

constexpr unsigned ExpectedStamp = 0x6af6fd2f;
constexpr unsigned ExpectedImageSize = 0x39f8000;
constexpr unsigned GateRva = 0x8e34e0;
constexpr unsigned char GateBytes[] = { 0x8b,0x91,0x30,0x02,0x00,0x00,0x48,0x8b,0x81,0x38,0x02,0x00,0x00,0x81,0xfa,0x00,0x00,0x01,0x00 };

inline bool IsSupportedBuild(unsigned stamp, unsigned imageSize,
                             const unsigned char* gate, std::size_t gateSize) {
    return stamp == ExpectedStamp && imageSize == ExpectedImageSize && gate &&
           gateSize == sizeof(GateBytes) && std::memcmp(gate, GateBytes, gateSize) == 0;
}
