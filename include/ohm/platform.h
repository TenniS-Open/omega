//
// Created by kier on 2020/7/22.
//

#ifndef OMEGA_PLATFORM_H
#define OMEGA_PLATFORM_H


/**
 * Platform
 * including
 *  OHM_PLATFORM_OS_WINDOWS
 *  OHM_PLATFORM_OS_MAC
 *  OHM_PLATFORM_OS_LINUX
 *  OHM_PLATFORM_OS_IOS
 *  OHM_PLATFORM_OS_ANDROID
 */
#if defined(__ANDROID__)
#   define OHM_PLATFORM_OS_ANDROID 1
#   define OHM_PLATFORM_OS_WINDOWS 0
#   define OHM_PLATFORM_OS_MAC     0
#   define OHM_PLATFORM_OS_LINUX   1
#   define OHM_PLATFORM_OS_IOS     0
#elif defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || \
    defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__)
#   define OHM_PLATFORM_OS_ANDROID 0
#   define OHM_PLATFORM_OS_WINDOWS 1
#   define OHM_PLATFORM_OS_MAC     0
#   define OHM_PLATFORM_OS_LINUX   0
#   define OHM_PLATFORM_OS_IOS     0
#elif defined(__MACOSX) || defined(__MACOS_CLASSIC__) || defined(__APPLE__) || defined(__apple__)
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#   define OHM_PLATFORM_OS_ANDROID 0
#   define OHM_PLATFORM_OS_WINDOWS 0
#   define OHM_PLATFORM_OS_MAC     0
#   define OHM_PLATFORM_OS_LINUX   0
#   define OHM_PLATFORM_OS_IOS     1
#elif TARGET_OS_MAC
#   define OHM_PLATFORM_OS_ANDROID 0
#   define OHM_PLATFORM_OS_WINDOWS 0
#   define OHM_PLATFORM_OS_MAC     1
#   define OHM_PLATFORM_OS_LINUX   0
#   define OHM_PLATFORM_OS_IOS     0
#else
//#   error "Unknown Apple platform"
#   define OHM_PLATFORM_OS_ANDROID 0
#   define OHM_PLATFORM_OS_WINDOWS 0
#   define OHM_PLATFORM_OS_MAC     0
#   define OHM_PLATFORM_OS_LINUX   0
#   define OHM_PLATFORM_OS_IOS     0
#endif
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || \
    defined(LINUX) || defined(_LINUX)
#   define OHM_PLATFORM_OS_ANDROID 0
#   define OHM_PLATFORM_OS_WINDOWS 0
#   define OHM_PLATFORM_OS_MAC     0
#   define OHM_PLATFORM_OS_LINUX   1
#   define OHM_PLATFORM_OS_IOS     0
#else
//#   error Unknown OS
#   define OHM_PLATFORM_OS_ANDROID 0
#   define OHM_PLATFORM_OS_WINDOWS 0
#   define OHM_PLATFORM_OS_MAC     0
#   define OHM_PLATFORM_OS_LINUX   0
#   define OHM_PLATFORM_OS_IOS     0

#endif

/**
 * System bits
 * including
 *  OHM_PLATFORM_BITS_32
 *  OHM_PLATFORM_BITS_64
 */
#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) || defined(__amd64) || \
    defined(__LP64__) || defined(_LP64) || defined(__x86_64__) || defined(__x86_64) || \
    defined(_M_X64) || defined(__ia64__) || defined(_IA64) || defined(__IA64__) || \
    defined(__ia64) || defined(_M_IA64)
#   define OHM_PLATFORM_BITS_16 0
#   define OHM_PLATFORM_BITS_32 0
#   define OHM_PLATFORM_BITS_64 1
#elif defined(_WIN32) || defined(WIN32) || defined(__32BIT__) || defined(__ILP32__) || \
    defined(_ILP32) || defined(i386) || defined(__i386__) || defined(__i486__) || \
    defined(__i586__) || defined(__i686__) || defined(__i386) || defined(_M_IX86) || \
    defined(__X86__) || defined(_X86_) || defined(__I86__)
#   define OHM_PLATFORM_BITS_16 0
#   define OHM_PLATFORM_BITS_32 1
#   define OHM_PLATFORM_BITS_64 0
#else
//#   error Unknown system bit-length
#   define OHM_PLATFORM_BITS_16 0
#   define OHM_PLATFORM_BITS_32 0
#   define OHM_PLATFORM_BITS_64 0
#endif

/**
 * Compiler
 * including
 *  OHM_PLATFORM_CC_MSVC
 *  OHM_PLATFORM_CC_MINGW
 *  OHM_PLATFORM_CC_GCC
 */
#if defined(_MSC_VER)
#   define OHM_PLATFORM_CC_MSVC  1
#   define OHM_PLATFORM_CC_MINGW 0
#   define OHM_PLATFORM_CC_GCC   0
#elif defined(__MINGW32__) || defined(__MINGW64__)
#   define OHM_PLATFORM_CC_MSVC  0
#   define OHM_PLATFORM_CC_MINGW 1
#   define OHM_PLATFORM_CC_GCC   1
#elif defined(__GNUG__) || defined(__GNUC__)
#   define OHM_PLATFORM_CC_MSVC  0
#   define OHM_PLATFORM_CC_MINGW 0
#   define OHM_PLATFORM_CC_GCC   1
#else
//#   error Unknown compiler
#   define OHM_PLATFORM_CC_MSVC  0
#   define OHM_PLATFORM_CC_MINGW 0
#   define OHM_PLATFORM_CC_GCC   0
#endif

#if defined(__x86_64__) || defined(__amd64__) || defined(_M_IX86) || \
    defined(_M_X64)
#define OHM_PLATFORM_IS_X86 1
#endif

#endif //OMEGA_PLATFORM_H
