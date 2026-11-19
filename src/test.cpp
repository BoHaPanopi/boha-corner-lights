#include "controller.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace corner;
    assert(Side(LeftMarker,0x2000)==1 && Side(RightMarker,0x2000)==2);
    assert(Side(.083f,0x2000)==0 && Side(LeftMarker,0x40)==0);
    active=true; electric=true; left=true; right=false; lastSample=1000;
    // Repeated reads throughout a blink cycle remain on, regardless of pulse phase.
    for (unsigned t=1000;t<2000;t++) { assert(Enabled(1,t)); assert(!Enabled(2,t)); }
    right=true; assert(Enabled(1,1500)&&Enabled(2,1500));
    left=false; assert(!Enabled(1,1500)&&Enabled(2,1500));
    electric=false; assert(!Enabled(2,1500));
    electric=true; assert(!Enabled(2,2600));
    active=false; assert(!Enabled(2,1500)); active=true;
    left=false; right=false;
    UpdateSteering(14.0f/450.0f, 0); assert(!Enabled(1,1500));
    UpdateSteering(16.0f/450.0f, 0); assert(Enabled(1,1500)&&!Enabled(2,1500));
    UpdateSteering(12.0f/450.0f, 0); assert(Enabled(1,1500));
    UpdateSteering(9.0f/450.0f, 0); assert(!Enabled(1,1500));
    UpdateSteering(-16.0f/450.0f,39.0f/3.6f); assert(Enabled(2,1500)&&!Enabled(1,1500));
    UpdateSteering(-16.0f/450.0f,40.0f/3.6f); assert(!Enabled(2,1500));
    right=true; assert(Enabled(2,1500)); right=false; // Blinker bypasses steering speed limit.
    UpdateSteering(0.5f,-5.0f/3.6f); assert(Enabled(1,1500)); // Low-speed reversing.
    UpdateSteering(NAN,0); assert(!Enabled(1,1500));
    UpdateSteering(0.5f,NAN); assert(!Enabled(1,1500));
    UpdateSteering(1.5f,0); assert(!Enabled(1,1500));
    lastSample=0; assert(!Enabled(2,1500));
    puts("PASS: blinkers/hazards, steering direction, hysteresis, speed boundary, reverse, invalid data, power gating, timeout and stock-source exclusion");
}
