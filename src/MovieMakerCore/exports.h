#pragma once
#ifndef MOVIECORE_EXPORTS_H
#define MOVIECORE_EXPORTS_H

#ifdef MOVIECORE_EXPORTS
    #define MOVIECORE_API __declspec(dllexport)
#else
    #define MOVIECORE_API __declspec(dllimport)
#endif

#ifdef MOVIECORE_EXPORTS
    #define RIBBON_API __declspec(dllexport)
#else
    #define RIBBON_API __declspec(dllimport)
#endif

#ifdef MOVIECORE_EXPORTS
    #define STORYBOARD_API __declspec(dllexport)
#else
    #define STORYBOARD_API __declspec(dllimport)
#endif

#ifdef MOVIECORE_EXPORTS
    #define DATASTRUCT_API __declspec(dllexport)
#else
    #define DATASTRUCT_API __declspec(dllimport)
#endif

#endif // MOVIECORE_EXPORTS_H
