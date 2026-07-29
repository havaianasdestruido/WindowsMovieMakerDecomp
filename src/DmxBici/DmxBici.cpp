#include <windows.h>

#define UNREFERENCED_PARAMETER(p) (p)

// Internal implementations - these get the stdcall decorated names (_AddStringToDataPoint@12 etc.)

extern "C" BOOL __stdcall BiciWrapper_AddStringToDataPoint(DWORD key, DWORD subKey, const wchar_t* value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(subKey); UNREFERENCED_PARAMETER(value); return FALSE; }

extern "C" BOOL __stdcall BiciWrapper_AddToAverage(DWORD key, DWORD value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(value); return FALSE; }

extern "C" BOOL __stdcall BiciWrapper_AddToDataPoint(DWORD key, DWORD subKey, DWORD value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(subKey); UNREFERENCED_PARAMETER(value); return FALSE; }

extern "C" void __stdcall BiciWrapper_AddToStream(DWORD key, const void* tuple)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(tuple); }

extern "C" LONG __stdcall BiciWrapper_EndExperience()
{ return 1; }

extern "C" BOOL __stdcall BiciWrapper_Increment(DWORD key, DWORD value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(value); return TRUE; }

extern "C" BOOL __stdcall BiciWrapper_Set(DWORD key, DWORD value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(value); return TRUE; }

extern "C" LONG __stdcall BiciWrapper_SetAnid(const wchar_t* anid)
{ UNREFERENCED_PARAMETER(anid); return 0x80004005; }

extern "C" BOOL __stdcall BiciWrapper_SetIfMax(DWORD key, DWORD value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(value); return TRUE; }

extern "C" BOOL __stdcall BiciWrapper_SetIfMin(DWORD key, DWORD value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(value); return TRUE; }

extern "C" BOOL __stdcall BiciWrapper_SetString(DWORD key, const wchar_t* value)
{ UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(value); return FALSE; }

extern "C" LONG __stdcall BiciWrapper_StartExperienceWithId(DWORD id)
{ UNREFERENCED_PARAMETER(id); return 1; }

extern "C" LONG __stdcall BiciWrapper_StartExperience()
{ return 0; }

extern "C" BOOL __stdcall BiciWrapper_TimerAccumulate(DWORD timerId)
{ UNREFERENCED_PARAMETER(timerId); return FALSE; }

extern "C" BOOL __stdcall BiciWrapper_TimerRecord(DWORD timerId)
{ UNREFERENCED_PARAMETER(timerId); return FALSE; }

extern "C" BOOL __stdcall BiciWrapper_TimerStart(DWORD timerId)
{ UNREFERENCED_PARAMETER(timerId); return TRUE; }

extern "C" BOOL __stdcall BiciWrapper_TransferExperienceToApp(wchar_t*** outNames)
{ UNREFERENCED_PARAMETER(outNames); return TRUE; }

extern "C" BOOL __stdcall BiciWrapper_TransferExperienceToAppId(DWORD appId)
{ UNREFERENCED_PARAMETER(appId); return TRUE; }

extern "C" BOOL __stdcall BiciWrapper_TransferExperienceToWeb(const wchar_t* url, wchar_t*** outParams)
{ UNREFERENCED_PARAMETER(url); UNREFERENCED_PARAMETER(outParams); return FALSE; }
