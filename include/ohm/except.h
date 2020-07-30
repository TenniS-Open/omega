//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_EXCEPT_H
#define OMEGA_EXCEPT_H

#include "platform.h"

#if OHM_PLATFORM_CC_MSVC
#define OHM_NOEXCEPT
#else
#define OHM_NOEXCEPT noexcept
#endif

#endif //OMEGA_EXCEPT_H
