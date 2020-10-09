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

#elif OHM_PLATFORM_OS_LINUX || OHM_PLATFORM_OS_MAC
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#error "socket only support in windows and unix link system."
#endif

namespace ohm {
    enum class SocketError : int32_t {
        // The operation completed successfully.
        SUCCESS = 0,
        // An invalid argument was supplied.
        INVAL = 0x1000,
        // An address incompatible with the requested protocol was used.
        AFNOSUPPORT = 0x1001,
        // The requested protocol has not been configured into the system, or no implementation for it exists.
        PROTONOSUPPORT = 0x1002,
        // A protocol was specified in the socket function call that does not support the semantics of the socket type requested.
        PROTOTYPE = 0x1003,
        // The support for the specified socket type does not exist in this address family.
        SOCKETNOSUPPORT = 0x1004,
        // An attempt was made to access a socket in a way forbidden by its access permissions.
        ACCES = 0x2001,
        // The attempted operation is not supported for the type of object referenced.
        OPNOTSUPP = 0x2002,
        // Only one usage of each socket address (protocol/network address/port) is normally permitted.
        ADDRINUSE = 0x3001,
        // The requested address is not valid in its context.
        ADDRNOTAVAIL = 0x3002,
        // A connect request was made on an already connected socket.
        ISCONN = 0x3003,
        // An established connection was aborted by the software in your host machine.
        CONNABORTED = 0x3004,
        // An existing connection was forcibly closed by the remote host.
        CONNRESET = 0x3005,
        // No connection could be made because the target machine actively refused it.
        CONNREFUSED = 0x3006,
        // An operation was attempted on a non-blocking socket that already had an operation in progress.
        ALREADY = 0x3007,
        // A socket operation was attempted to an unreachable network.
        NETUNREACH = 0x3008,
        // A socket operation was attempted to an unreachable host.
        HOSTUNREACH = 0x3009,
        // A connection attempt failed because the connected party did not properly respond after a period of time,
        // or established connection failed because connected host has failed to respond.
        TIMEDOUT = 0x300a,
        // A socket operation encountered a dead network.
        NETDOWN = 0x300b,
        // An operation was attempted on something that is not a socket.
        NOTSOCK = 0x300c,
        // A non-blocking socket operation could not be completed immediately.
        WOULDBLOCK = 0x4001,
        // A request to send or receive data was disallowed because the socket is not connected
        // and (when sending on a datagram socket using a sendto call) no address was supplied.
        NOTCONN = 0x4002,
        // A blocking operation was interrupted by a call to WSACancelBlockingCall.
        INTR = 0x4003,
        // A request to send or receive data was disallowed because the socket had already been shut down in
        // that direction with a previous shutdown call.
        SHUTDOWN = 0x4004,
        // A message sent on a datagram socket was larger than the internal message buffer or some other network limit,
        // or the buffer used to receive a datagram into was smaller than the datagram itself.
        MSGSIZE = 0x4005,
        // Resource temporarily unavailable
        AGAIN = 0x4006,
        // The system detected an invalid pointer address in attempting to use a pointer argument in a call.
        FAULT = 0xF000,
        // Too many open sockets.
        MFILE = 0xF002,
        // An operation on a socket could not be performed because the system lacked sufficient buffer space
        // or because a queue was full.
        NOBUFS = 0xF003,
        // Unknown error.
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

        inline SocketError tell_errcode() {
#if OHM_PLATFORM_OS_WINDOWS
            switch (WSAGetLastError()) {
                default: return SocketError::UNKNOWN;
                case ERROR_SUCCESS: return SocketError::SUCCESS;
                    // parameter error
                case WSAEINVAL: return SocketError::INVAL;
                case WSAEAFNOSUPPORT: return SocketError::AFNOSUPPORT;
                case WSAEPROTONOSUPPORT: return SocketError::PROTONOSUPPORT;
                case WSAEPROTOTYPE: return SocketError::PROTOTYPE;
                case WSAESOCKTNOSUPPORT: return SocketError::SOCKETNOSUPPORT;
                    // permission error
                case WSAEACCES: return SocketError::ACCES;
                case WSAEOPNOTSUPP: return SocketError::OPNOTSUPP;
                    // connection error
                case WSAEADDRINUSE: return SocketError::ADDRINUSE;
                case WSAEADDRNOTAVAIL: return SocketError::ADDRNOTAVAIL;
                case WSAEISCONN: return SocketError::ISCONN;
                case WSAECONNABORTED: return SocketError::CONNABORTED;
                case WSAECONNRESET: return SocketError::CONNRESET;
                case WSAECONNREFUSED: return SocketError::CONNREFUSED;
                case WSAEALREADY: return SocketError::ALREADY;
                case WSAENETUNREACH: return SocketError::NETUNREACH;
                case WSAEHOSTUNREACH: return SocketError::HOSTUNREACH;
                case WSAETIMEDOUT: return SocketError::TIMEDOUT;
                case WSAENETDOWN: return SocketError::NETDOWN;
                case WSAENOTSOCK: return SocketError::NOTSOCK;
                    // io error
                case WSAEWOULDBLOCK: return SocketError::WOULDBLOCK;
                case WSAENOTCONN: return SocketError::NOTCONN;
                case WSAEINTR: return SocketError::INTR;
                case WSAESHUTDOWN: return SocketError::SHUTDOWN;
                case WSAEMSGSIZE: return SocketError::MSGSIZE;
                    // fault error
                case WSAEFAULT: return SocketError::FAULT;
                case WSAEMFILE: return SocketError::MFILE;
                case WSAENOBUFS: return SocketError::NOBUFS;
            }
#else
            switch (errno) {
                default: return SocketError::UNKNOWN;
                case 0: return SocketError::SUCCESS;
                    // parameter error
                case EINVAL: return SocketError::INVAL;
                case EAFNOSUPPORT: return SocketError::AFNOSUPPORT;
                case EPROTONOSUPPORT: return SocketError::PROTONOSUPPORT;
                case EPROTOTYPE: return SocketError::PROTOTYPE;
                case ESOCKTNOSUPPORT: return SocketError::SOCKETNOSUPPORT;
                    // permission error
                case EACCES: return SocketError::ACCES;
                case EOPNOTSUPP: return SocketError::OPNOTSUPP;
                    // connection error
                case EADDRINUSE: return SocketError::ADDRINUSE;
                case EADDRNOTAVAIL: return SocketError::ADDRNOTAVAIL;
                case EISCONN: return SocketError::ISCONN;
                case ECONNABORTED: return SocketError::CONNABORTED;
                case ECONNRESET: return SocketError::CONNRESET;
                case ECONNREFUSED: return SocketError::CONNREFUSED;
                case EALREADY: return SocketError::ALREADY;
                case ENETUNREACH: return SocketError::NETUNREACH;
                case EHOSTUNREACH: return SocketError::HOSTUNREACH;
                case ETIMEDOUT: return SocketError::TIMEDOUT;
                case ENETDOWN: return SocketError::NETDOWN;
                case ENOTSOCK: return SocketError::NOTSOCK;
                    // io error
                case EWOULDBLOCK: return SocketError::WOULDBLOCK;
                case ENOTCONN: return SocketError::NOTCONN;
                case EINTR: return SocketError::INTR;
                case ESHUTDOWN: return SocketError::SHUTDOWN;
                case EMSGSIZE: return SocketError::MSGSIZE;
                case EAGAIN: return SocketError::AGAIN;
                    // fault error
                case EFAULT: return SocketError::FAULT;
                case EMFILE: return SocketError::MFILE;
                case ENOBUFS: return SocketError::NOBUFS;
            }
#endif
        }
    }

    inline std::pair<SocketError, std::string> GetLastSocketError(const std::string &title = "") {
        auto code = _::tell_errcode();
        return std::make_pair(code, _::make_system_socket_error_message(title));
    }
}

#endif //OMEGA_SOCKET_ERRNO_H
