//
// Created by kier on 2020/10/5.
//

#ifndef OMEGA_SOCKET_ERRNO_H
#define OMEGA_SOCKET_ERRNO_H

#include <string>

#include "format.h"
#include "platform.h"

#if OHM_PLATFORM_OS_WINDOWS
#include "ohm/sys/winsock2.h"
#define SOCKET_T SOCKET
#define CLOSE_SOCKET closesocket
#elif OHM_PLATFORM_OS_LINUX || OHM_PLATFORM_OS_MAC
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET_T int
#define CLOSE_SOCKET close
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#else
#error "socket only support in windows and unix link system."
#endif

namespace ohm {
    enum class SocketError : int32_t {
        SUCCESS = 0,
        INVALID_PARAMETER = 0x1000,
        AF_NO_SUPPORT = 0x1001,
        PROTO_NO_SUPPORT = 0x1002,
        PROTOTYPE = 0x1003,
        SOCKET_NO_SUPPORT = 0x1004,
        WOULD_BLOCK = 0x3001,
        UNKNOWN = -1,
    };

    namespace _ {
        inline std::string make_system_socket_error_message(const std::string &title = "") {
            std::ostringstream oss;
#if OHM_PLATFORM_OS_WINDOWS
            oss << title << "Windows/winsock2: " << win::format_message(WSAGetLastError());
#else
            oss << title << "Linux/socket: (" << errno << "): " << strerror(errno);
#endif
            return oss.str();
        }
    }

    inline std::pair<SocketError, std::string> GetLastSocketError(const std::string &title="") {
        SocketError code;
#if OHM_PLATFORM_OS_WINDOWS
        switch (WSAGetLastError()) {
            default:
                code = SocketError::UNKNOWN;
                break;
            case ERROR_SUCCESS:
                code = SocketError::SUCCESS;
                break;
            case WSAEINVAL:
                code = SocketError::INVALID_PARAMETER;
                break;
            case WSAEAFNOSUPPORT:
                code = SocketError::AF_NO_SUPPORT;
                break;
            case WSAEPROTONOSUPPORT:
                code = SocketError::PROTO_NO_SUPPORT;
                break;
            case WSAEPROTOTYPE:
                code = SocketError::PROTOTYPE;
                break;
            case WSAESOCKTNOSUPPORT:
                code = SocketError::SOCKET_NO_SUPPORT;
                break;
            case WSAEWOULDBLOCK:
                code = SocketError::WOULD_BLOCK;
                break;
        }
#else
        switch (errno) {
            default:
                code = SocketError::UNKNOWN;
                break;
            case 0:
                code = SocketError::SUCCESS;
                break;
            case EINVAL:
                code = SocketError::INVALID_PARAMETER;
                break;
            case EAFNOSUPPORT:
                code = SocketError::AF_NO_SUPPORT;
                break;
            case EPROTONOSUPPORT:
                code = SocketError::PROTO_NO_SUPPORT;
                break;
            case EPROTOTYPE:
                code = SocketError::PROTOTYPE;
                break;
            case EWOULDBLOCK:
                code = SocketError::WOULD_BLOCK;
                break;
        }
#endif
        return std::make_pair(code, _::make_system_socket_error_message(title));
    }
}

#endif //OMEGA_SOCKET_ERRNO_H
