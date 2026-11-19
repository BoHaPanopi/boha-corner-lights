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
    // Diagnostic probe: each distinct unknown (marker, mask) pair is reported once.
    assert(RecordUnknownSource(0.5f,0x40));
    assert(!RecordUnknownSource(0.5f,0x40));
    assert(RecordUnknownSource(0.5f,0x41));
    assert(!RecordUnknownSource(0.0f,0)); // sentinel pair is never reported
    // Probe v2.2: claims happen only inside a player light window, per-pair budgeted.
    int instA=0, instB=0;
    assert(lastPlayerLightChange.load()==0);
    assert(RecordSourceState(&instA,0.083f,0x1,true,5000)==0); // passive without any window
    assert(probeRejected.load()==0);                           // passive mode counts nothing
    lastPlayerLightChange.store(5000);
    assert(RecordSourceState(&instA,0.083f,0x1,false,5100)==0);// dark source claims no slot
    assert(RecordSourceState(&instA,0.083f,0x1,true,5000+ProbeWindowMs+1)==0); // window expired
    assert(probeRejected.load()==0);                           // outside window nothing is counted
    assert(RecordSourceState(&instA,0.083f,0x1,true,5500)==1); // lit inside window claims
    assert(RecordSourceState(&instA,0.083f,0x1,true,5600)==0); // repeat state stays silent
    assert(RecordSourceState(&instB,0.083f,0x1,true,5600)==1); // distinct instance, same pair
    assert(RecordSourceState(nullptr,0.083f,0x1,true,5500)==0);// null instance ignored
    bool litState=true; unsigned lastReport=1;
    for (int i=0;i<64;i++) {                                   // tracked transitions ignore window
        litState=!litState;
        if (const unsigned n=RecordSourceState(&instA,0.083f,0x1,litState,9000)) {
            assert(n==lastReport+1); lastReport=n;
        }
    }
    assert(lastReport==ProbeTransitionLimit);                  // per-instance budget enforced
    assert(RecordSourceState(&instA,0.083f,0x1,!litState,9000)==0);
    int budgetFill[14];
    for (auto& fill : budgetFill)                              // pair budget allows 16 in total
        assert(RecordSourceState(&fill,0.083f,0x1,true,5500)==1);
    int beyondBudget=0;
    assert(RecordSourceState(&beyondBudget,0.083f,0x1,true,5500)==0); // pair budget exhausted
    assert(probeRejected.load()==1);                           // rejection counted once for this second
    assert(RecordSourceState(&beyondBudget,0.083f,0x1,true,5600)==0);
    assert(probeRejected.load()==1);                           // repeats in the same second not recounted
    assert(RecordSourceState(&beyondBudget,0.083f,0x1,true,6400)==0);
    assert(probeRejected.load()==2);                           // next second counts again
    int fillers[112]; int claimed=0, index=0;
    for (int pair=0;pair<7;pair++)
        for (int k=0;k<16;k++)
            claimed += RecordSourceState(&fillers[index++],2.0f+static_cast<float>(pair),0x2,true,5500) ? 1 : 0;
    assert(claimed==112);                                      // table now holds all 128 instances
    lastPlayerLightChange.store(20000);
    int overflowInstance=0;
    assert(RecordSourceState(&overflowInstance,10.0f,0x4,true,20500)==0); // full table stays silent
    assert(probeRejected.load()==3);                           // full-table rejection counted
    assert(RecordSourceState(&instB,0.083f,0x1,false,50000)==2); // tracked instances keep reporting
    // Pair table bounded: 10 pairs are used above, the rest fills up and goes silent.
    int added=0;
    for (unsigned i=0;i<ProbeCapacity*2;i++)
        added += RecordUnknownSource(1.0f+static_cast<float>(i),0x2) ? 1 : 0;
    assert(added==static_cast<int>(ProbeCapacity)-10);
    assert(!RecordUnknownSource(123.0f,0x7));
    puts("PASS: profile routing, build fingerprint, blinkers/hazards, steering, speed, invalid data, power gating, stock-source exclusion, probe dedup and probe v2.2 windowed claims");
}
