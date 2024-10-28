#ifdef TE_INTERNAL

#ifndef TE_PROFILING_H
#define TE_PROFILING_H

#include "DataTypes.h"

#ifdef WIN32

#include <windows.h>
GlobalVariable u64 frequency;

inline void WIN32_InitProfiling() {
    LARGE_INTEGER frequencyLI;

    QueryPerformanceFrequency(&frequencyLI);

    frequency = (u64)frequencyLI.QuadPart;
}

inline f64 WIN32_GetTimeInSeconds() {
    LARGE_INTEGER counterLI;

    QueryPerformanceCounter(&counterLI);

    u64 counter = (u64)counterLI.QuadPart;
    f64 result = (f64)counter / (f64)frequency;

    return result;
}

inline u64 WIN32_GetCounter() {
    LARGE_INTEGER counterLI;

    QueryPerformanceCounter(&counterLI);

    u64 counter = (u64)counterLI.QuadPart;

    return counter;
}

#define GetCounter() WIN32_GetCounter()
#define GetTimeInSeconds() WIN32_GetTimeInSeconds()
#define InitProfiling() WIN32_InitProfiling()
#else
#endif

GlobalVariable f64 timeMark;
GlobalVariable u64 counterMark;

inline void BeginTimeMeasurement() {
    counterMark = GetCounter();
}

inline f32 ToSeconds(u64 counter) {
    return (f32)counter / (f32)frequency;
}

inline f32 EndTimeMeasurement() {
    u64 endCounterMark = GetCounter();
    f32 result = ToSeconds(endCounterMark - counterMark);

    return result;
}

inline f32 ToMiliseconds(f32 seconds) {
    return seconds * 1000.0f;
}

#endif //TE_PROFILING_H
#else //TE_INTERNAL
#define InitProfiling(...)
#define BeginTimeMeasurement(...)
#define EndTimeMeasurement(...)
#define ToMiliseconds(...) 0
#endif 
