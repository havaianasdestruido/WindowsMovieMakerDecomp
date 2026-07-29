#include <windows.h>
#include <map>
#include <string>
#include <vector>

namespace {

    struct ExperienceState {
        DWORD id = 0;
        LONG refCount = 0;
        bool active = false;
        std::wstring anid;
        std::map<DWORD, DWORD> values;
        std::map<DWORD, std::wstring> strings;
        std::map<DWORD, std::map<DWORD, std::wstring>> dataPoints;
        std::map<DWORD, DWORD> timerStarts;
        std::map<DWORD, DWORD> timerAccums;
        std::map<DWORD, DWORD> avgSums;
        std::map<DWORD, DWORD> avgCounts;
        std::vector<const void*> streamTuples;
    };

    ExperienceState g_exp;

    DWORD NextId()
    {
        static LONG counter = 100;
        return static_cast<DWORD>(InterlockedIncrement(&counter));
    }
}

extern "C" BOOL __stdcall BiciWrapper_AddStringToDataPoint(DWORD key, DWORD subKey, const wchar_t* value)
{
    if (value)
        g_exp.dataPoints[key][subKey] = value;
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_AddToAverage(DWORD key, DWORD value)
{
    g_exp.avgSums[key] += value;
    g_exp.avgCounts[key] += 1;
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_AddToDataPoint(DWORD key, DWORD subKey, DWORD value)
{
    g_exp.dataPoints[key][subKey] = std::to_wstring(value);
    return TRUE;
}

extern "C" void __stdcall BiciWrapper_AddToStream(DWORD key, const void* tuple)
{
    g_exp.streamTuples.push_back(tuple);
}

extern "C" LONG __stdcall BiciWrapper_EndExperience()
{
    if (g_exp.refCount > 0)
        --g_exp.refCount;
    if (g_exp.refCount == 0)
        g_exp.active = false;
    return static_cast<LONG>(g_exp.refCount);
}

extern "C" BOOL __stdcall BiciWrapper_Increment(DWORD key, DWORD value)
{
    g_exp.values[key] += value;
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_Set(DWORD key, DWORD value)
{
    g_exp.values[key] = value;
    return TRUE;
}

extern "C" LONG __stdcall BiciWrapper_SetAnid(const wchar_t* anid)
{
    if (anid)
        g_exp.anid = anid;
    return S_OK;
}

extern "C" BOOL __stdcall BiciWrapper_SetIfMax(DWORD key, DWORD value)
{
    auto it = g_exp.values.find(key);
    if (it == g_exp.values.end() || value > it->second)
        g_exp.values[key] = value;
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_SetIfMin(DWORD key, DWORD value)
{
    auto it = g_exp.values.find(key);
    if (it == g_exp.values.end() || value < it->second)
        g_exp.values[key] = value;
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_SetString(DWORD key, const wchar_t* value)
{
    if (value)
        g_exp.strings[key] = value;
    return TRUE;
}

extern "C" LONG __stdcall BiciWrapper_StartExperienceWithId(DWORD id)
{
    g_exp = {};
    g_exp.id = id;
    g_exp.active = true;
    g_exp.refCount = 1;
    return static_cast<LONG>(id);
}

extern "C" LONG __stdcall BiciWrapper_StartExperience()
{
    DWORD id = NextId();
    g_exp = {};
    g_exp.id = id;
    g_exp.active = true;
    g_exp.refCount = 1;
    return static_cast<LONG>(id);
}

extern "C" BOOL __stdcall BiciWrapper_TimerAccumulate(DWORD timerId)
{
    DWORD now = GetTickCount();
    auto it = g_exp.timerStarts.find(timerId);
    if (it != g_exp.timerStarts.end()) {
        g_exp.timerAccums[timerId] += (now - it->second);
        it->second = now;
    }
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_TimerRecord(DWORD timerId)
{
    DWORD now = GetTickCount();
    auto it = g_exp.timerStarts.find(timerId);
    if (it != g_exp.timerStarts.end()) {
        g_exp.timerAccums[timerId] += (now - it->second);
        g_exp.timerStarts.erase(it);
    }
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_TimerStart(DWORD timerId)
{
    g_exp.timerStarts[timerId] = GetTickCount();
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_TransferExperienceToApp(wchar_t*** outNames)
{
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_TransferExperienceToAppId(DWORD appId)
{
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_TransferExperienceToWeb(const wchar_t* url, wchar_t*** outParams)
{
    if (url) {
        OutputDebugStringW(L"[DmxBici] TransferExperienceToWeb: ");
        OutputDebugStringW(url);
        OutputDebugStringW(L"\n");
    }
    return TRUE;
}
