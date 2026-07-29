#include <windows.h>

#define UNREFERENCED_PARAMETER(p) (p)

// Lifecycle (3)
extern "C" void __stdcall Sqm_Startup() {}
extern "C" void __stdcall Sqm_StartupWithAppId(DWORD appId) { UNREFERENCED_PARAMETER(appId); }
extern "C" void __stdcall Sqm_Shutdown() {}

// Configuration (6)
extern "C" void __stdcall Sqm_SetAppStatusReportingMode(BOOL mode) { UNREFERENCED_PARAMETER(mode); }
extern "C" void __stdcall Sqm_SetOptInPreference(BOOL optIn) { UNREFERENCED_PARAMETER(optIn); }
extern "C" void __stdcall Sqm_SetApplicationMode(DWORD mode) { UNREFERENCED_PARAMETER(mode); }
extern "C" void __stdcall Sqm_SetAppDefinedValue(DWORD value) { UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_EnableShipAsserts() {}
extern "C" void __stdcall Sqm_InitializeUserExecutedActionReporting(DWORD a, DWORD b) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); }

// Query (2)
extern "C" DWORD __stdcall Sqm_GetOptInState() { return 0; }
extern "C" BOOL __stdcall Sqm_IsEnabled() { return FALSE; }

// Data Collection - Basic Setters (5)
extern "C" void __stdcall Sqm_Set(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_SetString(DWORD id, const wchar_t* value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_SetIfMin(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_SetIfMax(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_Increment(DWORD id, DWORD increment) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(increment); }

// Data Collection - Streams (5)
extern "C" void __stdcall Sqm_AddToStream(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_AddToStream3(DWORD id, DWORD a, DWORD b) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); }
extern "C" void __stdcall Sqm_AddToStream4(DWORD id, DWORD a, DWORD b, DWORD c) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); UNREFERENCED_PARAMETER(c); }
extern "C" void __stdcall Sqm_AddToStreamTuple(DWORD id, const void* tuple) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(tuple); }
extern "C" void __stdcall Sqm_AddToStreamW(DWORD id, const wchar_t* value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }

// Averages (1)
extern "C" void __stdcall Sqm_AddToAverage(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }

// Deferred Operations (8)
extern "C" void __stdcall Sqm_DeferSetIfMin(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_DeferSetIfMax(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_DeferAddToAverage(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_DeferAddToMedian(DWORD id, DWORD value) { UNREFERENCED_PARAMETER(id); UNREFERENCED_PARAMETER(value); }
extern "C" void __stdcall Sqm_DeferReportMin(DWORD id) { UNREFERENCED_PARAMETER(id); }
extern "C" void __stdcall Sqm_DeferReportMax(DWORD id) { UNREFERENCED_PARAMETER(id); }
extern "C" void __stdcall Sqm_DeferReportAverage(DWORD id) { UNREFERENCED_PARAMETER(id); }
extern "C" void __stdcall Sqm_DeferReportMedian(DWORD id) { UNREFERENCED_PARAMETER(id); }

// Timers (2)
extern "C" void __stdcall Sqm_StartTimer(DWORD id) { UNREFERENCED_PARAMETER(id); }
extern "C" void __stdcall Sqm_PauseTimer(DWORD id) { UNREFERENCED_PARAMETER(id); }

// Stream Timers (9)
extern "C" void __stdcall Sqm_StartStreamTimer(DWORD a, DWORD b, DWORD c) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); UNREFERENCED_PARAMETER(c); }
extern "C" void __stdcall Sqm_StopStreamTimer(DWORD a, DWORD b) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); }
extern "C" void __stdcall Sqm_AbortStreamTimer(DWORD a, DWORD b) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); }
extern "C" BOOL __stdcall Sqm_IsStreamTimerActive(DWORD a, DWORD b) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); return FALSE; }
extern "C" BOOL __stdcall Sqm_IsStreamTimerDataSet(DWORD a, DWORD b) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); return FALSE; }
extern "C" void __stdcall Sqm_AddStreamTimerData3(DWORD a, DWORD b, DWORD c) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); UNREFERENCED_PARAMETER(c); }
extern "C" void __stdcall Sqm_AddStreamTimerDataW(DWORD a, DWORD b, const wchar_t* c) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); UNREFERENCED_PARAMETER(c); }
extern "C" void __stdcall Sqm_AddToStreamTimer4(DWORD a, DWORD b, DWORD c, const void* tuple) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); UNREFERENCED_PARAMETER(c); UNREFERENCED_PARAMETER(tuple); }
extern "C" void __stdcall Sqm_AddToStreamTimer3(DWORD a, DWORD b, const void* tuple) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); UNREFERENCED_PARAMETER(tuple); }

// User Action Reporting (3)
extern "C" void __stdcall Sqm_ReportUserExecutedAction(DWORD a, DWORD b) { UNREFERENCED_PARAMETER(a); UNREFERENCED_PARAMETER(b); }
extern "C" void __stdcall Sqm_ReportAppLaunchStatus(BOOL success) { UNREFERENCED_PARAMETER(success); }
extern "C" void __stdcall Sqm_ReportAppCloseStatus(BOOL success) { UNREFERENCED_PARAMETER(success); }
