//
// Created by kier on 2020/9/7.
//

#ifndef OMEGA_WINDOWS_H
#define OMEGA_WINDOWS_H

#include "../platform.h"
#include <iomanip>

#if OHM_PLATFORM_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace ohm {
    namespace win {
        inline std::string format_message(DWORD dw) {
            LPVOID lpMsgBuf = nullptr;
            ::FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dw,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) & lpMsgBuf,
                    0, NULL);
            std::ostringstream msg;
            msg << "(" << std::hex << "0x" << std::setw(8) << std::setfill('0') << dw << "): ";
            if (lpMsgBuf != nullptr) {
                msg << std::string(reinterpret_cast<char *>(lpMsgBuf));
                LocalFree(lpMsgBuf);
            }
            return msg.str();
        }
    }
}

/**
 * Undefined use variable in omega
 */
#undef VOID
#undef min
#undef max

#endif

#endif //OMEGA_WINDOWS_H
