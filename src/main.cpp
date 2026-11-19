#include <windows.h>
#include <atomic>
#include <cstdio>
#include <cstdarg>
#include <cstring>
extern "C" {
#include <scssdk_telemetry.h>
#include <common/scssdk_telemetry_truck_common_channels.h>
}
#include <MinHook.h>
#include "controller.h"
#include "fingerprint.h"

namespace {
using Gate = bool(__fastcall*)(void*);
Gate original = nullptr;
void* hooked = nullptr;
FILE* logFile = nullptr;
std::atomic<unsigned> seenLeft{0}, seenRight{0};
scs_log_t gameLog = nullptr;
void Log(const char* fmt, ...) {
    if (!logFile) return;
    va_list args; va_start(args,fmt); vfprintf(logFile,fmt,args); va_end(args);
    fputc('\n',logFile); fflush(logFile);
}
bool __fastcall Check(void* self) {
    int side = 0;
    float marker = 0.0f;
    unsigned mask = 0;
    __try {
        auto* p = static_cast<unsigned char*>(self);
        marker = *reinterpret_cast<float*>(p+0x104);
        mask = *reinterpret_cast<unsigned*>(p+0x230);
        side = corner::Side(marker, mask);
    } __except(EXCEPTION_EXECUTE_HANDLER) { return original(self); }
    if (side) {
        auto& count = side == 1 ? seenLeft : seenRight;
        if (count.fetch_add(1) == 0) Log("Recognized custom %s corner source",side==1?"left":"right");
        return corner::Enabled(side,GetTickCount64());
    }
    // Unknown sources keep stock behavior; log each distinct pair once, then
    // report state transitions per instance (probe v2.2) within fixed budgets.
    if (corner::RecordUnknownSource(marker, mask))
        Log("Probe: unknown flare source marker=0x%08x mask=0x%08x",corner::Bits(marker),mask);
    const bool lit = original(self);
    const unsigned long long now = GetTickCount64();
    if (const unsigned report = corner::RecordSourceState(self, marker, mask, lit, now))
        Log("Probe2 t=%llu inst=%p marker=0x%08x mask=0x%08x lit=%d n=%u",
            now, self, corner::Bits(marker), mask, lit ? 1 : 0, report);
    return lit;
}
// Player light channels open the probe claim window; index via context cookie.
const char* const lightChannelNames[] = {
    SCS_TELEMETRY_TRUCK_CHANNEL_light_parking,
    SCS_TELEMETRY_TRUCK_CHANNEL_light_low_beam,
    SCS_TELEMETRY_TRUCK_CHANNEL_light_high_beam,
    SCS_TELEMETRY_TRUCK_CHANNEL_light_aux_front,
};
std::atomic<int> lightStates[4]{{-1},{-1},{-1},{-1}};
SCSAPI_VOID LightChannel(const scs_string_t, const scs_u32_t,
                         const scs_value_t* v, const scs_context_t context) {
    const int index = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (index < 0 || index >= 4 || !v) return;
    int value = -1;
    if (v->type == SCS_VALUE_TYPE_bool) value = v->value_bool.value ? 1 : 0;
    else if (v->type == SCS_VALUE_TYPE_u32) value = static_cast<int>(v->value_u32.value);
    if (value < 0) return;
    const int previous = lightStates[index].exchange(value);
    if (previous == value) return;
    // The initial delivery is not a player action and must not open a window.
    if (previous == -1) return;
    const unsigned long long now = GetTickCount64();
    corner::lastPlayerLightChange.store(now);
    Log("Light %s=%d t=%llu",lightChannelNames[index],value,now);
}
SCSAPI_VOID Channel(const scs_string_t name, const scs_u32_t,
                   const scs_value_t* v, const scs_context_t) {
    if (!std::strcmp(name,"truck.effective.steering") || !std::strcmp(name,"truck.speed")) {
        const float number = v && v->type == SCS_VALUE_TYPE_float ?
            v->value_float.value : std::numeric_limits<float>::quiet_NaN();
        if (!std::strcmp(name,"truck.speed")) corner::speed.store(number);
        else corner::steering.store(number);
        return;
    }
    const bool value = v && v->type == SCS_VALUE_TYPE_bool && v->value_bool.value;
    if (!std::strcmp(name,"truck.lblinker")) corner::left.store(value);
    if (!std::strcmp(name,"truck.rblinker")) corner::right.store(value);
    if (!std::strcmp(name,"truck.electric.enabled")) corner::electric.store(value);
    Log("State %s=%d",name,value);
}
SCSAPI_VOID Frame(const scs_event_t, const void*, const scs_context_t) {
    const bool beforeLeft=corner::steerLeft.load(), beforeRight=corner::steerRight.load();
    corner::UpdateSteering(corner::steering.load(),corner::speed.load());
    if (beforeLeft!=corner::steerLeft.load() || beforeRight!=corner::steerRight.load())
        Log("Steering lights left=%d right=%d steering=%.3f speed_kmh=%.1f",
            corner::steerLeft.load(),corner::steerRight.load(),corner::steering.load(),corner::speed.load()*3.6f);
    corner::lastSample.store(GetTickCount64());
}
SCSAPI_VOID State(const scs_event_t event, const void*, const scs_context_t) {
    corner::active.store(event == SCS_TELEMETRY_EVENT_started);
}
bool Install() {
    auto* module = reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));
    const auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(module);
    const auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(module+dos->e_lfanew);
    // Supported executable fingerprint and exact gate bytes are generated offline.
    auto* gate = module + GateRva;
    if (!IsSupportedBuild(nt->FileHeader.TimeDateStamp, nt->OptionalHeader.SizeOfImage,
                          gate, sizeof(GateBytes))) {
        Log("Unsupported ETS2 build or native gate signature: no hooks installed"); return false;
    }
    if (MH_Initialize() != MH_OK) return false;
    if (MH_CreateHook(gate, &Check, reinterpret_cast<void**>(&original)) != MH_OK) return false;
    if (MH_EnableHook(gate) != MH_OK) return false;
    hooked=gate;
    Log("Native flare gate installed; only private source markers are overridden");
    return true;
}
}
extern "C" SCSAPI_RESULT scs_telemetry_init(
    const scs_u32_t version, const scs_telemetry_init_params_t* params) {
    if (version != SCS_TELEMETRY_VERSION_1_01) return SCS_RESULT_unsupported;
    const auto* p=reinterpret_cast<const scs_telemetry_init_params_v101_t*>(params);
    wchar_t dll[MAX_PATH]; HMODULE own=nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      reinterpret_cast<LPCWSTR>(&scs_telemetry_init),&own);
    GetModuleFileNameW(own,dll,MAX_PATH);
    if (auto* slash=wcsrchr(dll,L'\\')) wcscpy_s(slash+1,MAX_PATH-(slash+1-dll),L"BoHaCornerLights.log");
    _wfopen_s(&logFile,dll,L"w");
    Log("BoHa Corner Lights 1.1 (probe v2.2) - blinkers + steering 15/10 deg, below 40 km/h, 900-deg wheel");
    if (!Install()) { if(logFile){fclose(logFile);logFile=nullptr;} return SCS_RESULT_unsupported; }
    const char* channels[]={"truck.lblinker","truck.rblinker","truck.electric.enabled"};
    for (auto* c:channels) {
        if (p->register_for_channel(c,SCS_U32_NIL,SCS_VALUE_TYPE_bool,
            SCS_TELEMETRY_CHANNEL_FLAG_none,Channel,nullptr)!=SCS_RESULT_ok) {
            MH_DisableHook(hooked); MH_RemoveHook(hooked); hooked=nullptr;
            Log("Required channel unavailable: %s; disabled",c); return SCS_RESULT_unsupported;
        }
    }
    const char* steeringChannels[]={"truck.effective.steering", "truck.speed"};
    for (const auto* c : steeringChannels) {
        if (p->register_for_channel(c,SCS_U32_NIL,SCS_VALUE_TYPE_float,
            SCS_TELEMETRY_CHANNEL_FLAG_none,Channel,nullptr)!=SCS_RESULT_ok)
            Log("Steering channel unavailable: %s; blinker control remains available",c);
    }
    // Optional player light channels drive the probe claim window. Without any
    // of them the window never opens and the instance probe stays passive.
    const scs_value_type_t lightTypes[4]={SCS_VALUE_TYPE_bool,SCS_VALUE_TYPE_bool,
                                          SCS_VALUE_TYPE_bool,SCS_VALUE_TYPE_u32};
    unsigned lightChannels=0;
    for (int i=0;i<4;i++) {
        if (p->register_for_channel(lightChannelNames[i],SCS_U32_NIL,lightTypes[i],
            SCS_TELEMETRY_CHANNEL_FLAG_none,LightChannel,
            reinterpret_cast<scs_context_t>(static_cast<intptr_t>(i)))==SCS_RESULT_ok)
            lightChannels++;
        else
            Log("Light channel unavailable: %s",lightChannelNames[i]);
    }
    if (!lightChannels)
        Log("No light channels; instance probe stays passive, pair probe remains active");
    p->register_for_event(SCS_TELEMETRY_EVENT_frame_end,Frame,nullptr);
    p->register_for_event(SCS_TELEMETRY_EVENT_paused,State,nullptr);
    p->register_for_event(SCS_TELEMETRY_EVENT_started,State,nullptr);
    p->common.log(SCS_LOG_TYPE_message,"[BoHaCornerLights 1.1] initialized; logical blinkers + steering below 40 km/h");
    return SCS_RESULT_ok;
}
extern "C" SCSAPI_VOID scs_telemetry_shutdown() {
    corner::lastSample.store(0);
    if (hooked) { MH_DisableHook(hooked); MH_RemoveHook(hooked); hooked=nullptr; }
    unsigned tracked=0;
    for (const auto& slot : corner::probeInstances)
        if (slot.instance.load(std::memory_order_relaxed)) tracked++;
    Log("Probe summary: tracked=%u rejected_seconds=%u",tracked,corner::probeRejected.load());
    Log("Shutdown; custom source calls left=%u right=%u",seenLeft.load(),seenRight.load());
    if(logFile){fclose(logFile);logFile=nullptr;}
}
