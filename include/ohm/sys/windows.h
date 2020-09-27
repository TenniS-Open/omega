//
// Created by kier on 2020/9/7.
//

#ifndef OMEGA_WINDOWS_H
#define OMEGA_WINDOWS_H

#include "../platform.h"

#if OHM_PLATFORM_OS_WINDOWS

#include <Windows.h>

/**
 * Undefined use variable in omega
 */
#undef VOID
#undef min
#undef max

#endif

#endif //OMEGA_WINDOWS_H
