#include "controller.h"
#include "fingerprint.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace corner;
    int profileSide=0;
    const auto* profile=FindProfile(Profiles[0].leftMarker,0x2000,profileSide);
    assert(profile==&Profiles[0] && profileSide==1);
    profile=FindProfile(Profiles[0].rightMarker,0x2000,profileSide);
    assert(profile==&Profiles[0] && profileSide==2);
    assert(std::strcmp(Profiles[0].id,"volvo-9700-dd-enrique-front-slots")==0);
    assert(std::strcmp(Profiles[0].modelAdapter,
                       "vehicle/bus/upgrade/enrique_9700/ec_fpanel.pmg")==0);
    assert(Profiles[0].steeringOnDegrees==15.0f &&
           Profiles[0].steeringOffDegrees==10.0f &&
           Profiles[0].steeringMaxSpeedKmh==40.0f);
    assert(Profiles[0].lightRangeMeters==25.0f &&
           Profiles[0].innerAngleDegrees==45.0f &&
           Profiles[0].outerAngleDegrees==110.0f &&
           Profiles[0].yawDegrees==50.0f && Profiles[0].pitchDegrees==-7.0f);
    assert(Side(.083f,0x2000)==0 && Side(Profiles[0].leftMarker,0x40)==0);
    assert(FindProfile(.083f,0x2000,profileSide)==nullptr && profileSide==0);
    assert(IsSupportedBuild(ExpectedStamp,ExpectedImageSize,GateBytes,sizeof(GateBytes)));
    assert(!IsSupportedBuild(ExpectedStamp+1,ExpectedImageSize,GateBytes,sizeof(GateBytes)));
    assert(!IsSupportedBuild(ExpectedStamp,ExpectedImageSize+1,GateBytes,sizeof(GateBytes)));
    unsigned char wrongGate[sizeof(GateBytes)]{};
    assert(!IsSupportedBuild(ExpectedStamp,ExpectedImageSize,wrongGate,sizeof(wrongGate)));
    assert(!IsSupportedBuild(ExpectedStamp,ExpectedImageSize,nullptr,sizeof(GateBytes)));
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
    puts("PASS: profile routing, build fingerprint, blinkers/hazards, steering, speed, invalid data, power gating and stock-source exclusion");
}
