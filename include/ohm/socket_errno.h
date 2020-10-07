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
        // parameter error
        INVALID_PARAMETER = 0x1000, ///< same as EINVAL
        AF_NO_SUPPORT = 0x1001,
        PROTO_NO_SUPPORT = 0x1002,
        PROTOTYPE = 0x1003,
        SOCKET_NO_SUPPORT = 0x1004,
        // permission error
        ACCESS_DENIED = 0x2001, ///< same as EACCES
        OPERATION_NOT_SUPPORTED = 0x2002,   ///< same as EOPNOTSUPP
        // connection error
        ADDRESS_IS_USED = 0x3001,    ///< same as EADDRINUSE
        ADDRESS_NOT_AVAILABLE = 0x3002, ///< same as EADDRNOTAVAIL
        IS_CONNECTED = 0x3003,  ///< same as EISCONN
        CONNECTION_ABORTED = 0x3004,  ///< same as ECONNABORTED
        CONNECTION_RESET = 0x3005,  ///< same as ECONNRESET
        CONNECTION_ALREADY = 0x3006,    ///< same as EALREADY
        CONNECTION_REFUSED = 0x3007,   ///< same as ECONNREFUSED
        NET_UNREACH = 0x3008,   ///< same as NETUNREACH
        HOST_UNREACH = 0x3009,  ///< same as HOSTUNREACH
        TIMEOUT = 0x300a,
        // io error
        WOULD_BLOCK = 0x3001,
        // fault error
        FAULT = 0xF000,
        NET_DOWN = 0xF001,  // same as ENETDOWN
        MFILE = 0xF002, // no more fd
        NOBUFS = 0xF003,    // no more buffer available
        // unknown error
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
