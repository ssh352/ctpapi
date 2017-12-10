#ifndef BFT_CORE_CORE_EXPORT_H
#define BFT_CORE_CORE_EXPORT_H

#if defined(WIN32)

#if defined(CORE_IMPLEMENTATION)
#define CORE_EXPORT __declspec(dllexport)
#define CORE_EXPORT_PRIVATE __declspec(dllexport)
#else
#define CORE_EXPORT __declspec(dllimport)
#define CORE_EXPORT_PRIVATE __declspec(dllimport)
#endif  // defined(CORE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(CORE_IMPLEMENTATION)
#define CORE_EXPORT __attribute__((visibility("default")))
#define CORE_EXPORT_PRIVATE __attribute__((visibility("default")))
#else
#define CORE_EXPORT
#define CORE_EXPORT_PRIVATE
#endif  // defined(CORE_IMPLEMENTATION)
#endif


#endif // BFT_CORE_CORE_EXPORT_H



