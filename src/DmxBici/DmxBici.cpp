#include <windows.h>
#include <map>
#include <string>
#include <vector>

namespace {

    static SRWLOCK g_expLock = {};

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
    AcquireSRWLockExclusive(&g_expLock);
    if (value)
        g_exp.dataPoints[key][subKey] = value;
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_AddToAverage(DWORD key, DWORD value)
{
    AcquireSRWLockExclusive(&g_expLock);
    g_exp.avgSums[key] += value;
    g_exp.avgCounts[key] += 1;
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_AddToDataPoint(DWORD key, DWORD subKey, DWORD value)
{
    AcquireSRWLockExclusive(&g_expLock);
    g_exp.dataPoints[key][subKey] = std::to_wstring(value);
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" void __stdcall BiciWrapper_AddToStream(DWORD key, const void* tuple)
{
    AcquireSRWLockExclusive(&g_expLock);
    g_exp.streamTuples.push_back(tuple);
    ReleaseSRWLockExclusive(&g_expLock);
}

extern "C" LONG __stdcall BiciWrapper_EndExperience()
{
    AcquireSRWLockExclusive(&g_expLock);
    if (g_exp.refCount > 0)
        --g_exp.refCount;
    if (g_exp.refCount == 0)
        g_exp.active = false;
    LONG result = static_cast<LONG>(g_exp.refCount);
    ReleaseSRWLockExclusive(&g_expLock);
    return result;
}

extern "C" BOOL __stdcall BiciWrapper_Increment(DWORD key, DWORD value)
{
    AcquireSRWLockExclusive(&g_expLock);
    g_exp.values[key] += value;
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_Set(DWORD key, DWORD value)
{
    AcquireSRWLockExclusive(&g_expLock);
    g_exp.values[key] = value;
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" LONG __stdcall BiciWrapper_SetAnid(const wchar_t* anid)
{
    AcquireSRWLockExclusive(&g_expLock);
    if (anid)
        g_exp.anid = anid;
    ReleaseSRWLockExclusive(&g_expLock);
    return S_OK;
}

extern "C" BOOL __stdcall BiciWrapper_SetIfMax(DWORD key, DWORD value)
{
    AcquireSRWLockExclusive(&g_expLock);
    auto it = g_exp.values.find(key);
    if (it == g_exp.values.end() || value > it->second)
        g_exp.values[key] = value;
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_SetIfMin(DWORD key, DWORD value)
{
    AcquireSRWLockExclusive(&g_expLock);
    auto it = g_exp.values.find(key);
    if (it == g_exp.values.end() || value < it->second)
        g_exp.values[key] = value;
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_SetString(DWORD key, const wchar_t* value)
{
    AcquireSRWLockExclusive(&g_expLock);
    if (value)
        g_exp.strings[key] = value;
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" LONG __stdcall BiciWrapper_StartExperienceWithId(DWORD id)
{
    AcquireSRWLockExclusive(&g_expLock);
    g_exp = {};
    g_exp.id = id;
    g_exp.active = true;
    g_exp.refCount = 1;
    ReleaseSRWLockExclusive(&g_expLock);
    return static_cast<LONG>(id);
}

extern "C" LONG __stdcall BiciWrapper_StartExperience()
{
    DWORD id = NextId();
    AcquireSRWLockExclusive(&g_expLock);
    g_exp = {};
    g_exp.id = id;
    g_exp.active = true;
    g_exp.refCount = 1;
    ReleaseSRWLockExclusive(&g_expLock);
    return static_cast<LONG>(id);
}

extern "C" BOOL __stdcall BiciWrapper_TimerAccumulate(DWORD timerId)
{
    AcquireSRWLockExclusive(&g_expLock);
    DWORD now = GetTickCount();
    auto it = g_exp.timerStarts.find(timerId);
    if (it != g_exp.timerStarts.end()) {
        g_exp.timerAccums[timerId] += (now - it->second);
        it->second = now;
    }
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_TimerRecord(DWORD timerId)
{
    AcquireSRWLockExclusive(&g_expLock);
    DWORD now = GetTickCount();
    auto it = g_exp.timerStarts.find(timerId);
    if (it != g_exp.timerStarts.end()) {
        g_exp.timerAccums[timerId] += (now - it->second);
        g_exp.timerStarts.erase(it);
    }
    ReleaseSRWLockExclusive(&g_expLock);
    return TRUE;
}

extern "C" BOOL __stdcall BiciWrapper_TimerStart(DWORD timerId)
{
    AcquireSRWLockExclusive(&g_expLock);
    g_exp.timerStarts[timerId] = GetTickCount();
    ReleaseSRWLockExclusive(&g_expLock);
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
