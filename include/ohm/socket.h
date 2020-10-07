//
// Created by kier on 2020/9/28.
//

#ifndef OMEGA_SOCKET_H
#define OMEGA_SOCKET_H

#include <stdexcept>
#include <exception>
#include <string>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <iomanip>

#include "byte_order.h"
#include "socket_errno.h"

#include "format.h"
#include "platform.h"
#include "need.h"
#include "except.h"

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
    class SocketException : public Exception {
    public:
        using self = SocketException;
        using supper = Exception;

        SocketException(std::string msg)
                : supper(Message(SocketError::UNKNOWN, std::move(msg))), m_errcode(SocketError::UNKNOWN) {}

        SocketException(SocketError errcode, std::string msg)
                : supper(Message(errcode, std::move(msg))), m_errcode(errcode) {}

        SocketException(const std::pair<SocketError, std::string> &error)
                : supper(Message(error.first, error.second)), m_errcode(error.first) {}

        SocketError errcode() const { return m_errcode; }

    public:
        static std::string Message(SocketError errcode, std::string msg) {
            std::ostringstream oss;
            oss << "Socket wrapper error";
            oss << "(" << std::hex << "0x" << std::setw(8) << std::setfill('0') << int(errcode) << "): ";
            oss << msg;
            return oss.str();
        }

    private:
        SocketError m_errcode = SocketError::UNKNOWN;
    };

    class SocketIOException : public SocketException {
    public:
        using self = SocketIOException;
        using supper = SocketException;

        using supper::supper;
    };

    inline std::string GetSystemSocketMessage() {
#if OHM_PLATFORM_OS_WINDOWS
        return win::format_message(WSAGetLastError());
#else
        return concat("(", errno, "): ", strerror(errno));
#endif
    }


    namespace _ {
        inline bool SocketWouldBlock() {
#if OHM_PLATFORM_OS_WINDOWS
            return WSAGetLastError() == WSAEWOULDBLOCK;
#else
            return errno == EWOULDBLOCK;
#endif
        }

        inline bool SocketEOF(SOCKET_T socket) {
            char mark;
            auto flag = ::recv(socket, &mark, 1, MSG_PEEK);
            if (flag > 0) return false;
            if (flag < 0 && SocketWouldBlock()) return false;
            return true;
        }
    }

    class SocketEnv {
    public:
        using self = SocketEnv;

        SocketEnv(const SocketEnv &) = delete;

        SocketEnv &operator=(const SocketEnv &) = delete;

        SocketEnv(SocketEnv &&) = default;

        SocketEnv &operator=(SocketEnv &&) = default;

#if OHM_PLATFORM_OS_WINDOWS

        SocketEnv() {
            WORD wVersionRequested;
            WSADATA wsaData;
            int err;
            wVersionRequested = MAKEWORD(2, 2);
            err = WSAStartup(wVersionRequested, &wsaData);
            if (err != 0) {
                throw SocketException(concat("WSAError: Can not startup socket 2.2. ",
                                             win::format_message(WSAGetLastError())));
            }
        }

        ~SocketEnv() {
            WSACleanup();
        }

#else
        SocketEnv() = default;
        ~SocketEnv() = default;
#endif
    };

    enum class Family : int {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
    };

    enum class Protocol : int {
        TCP = IPPROTO_TCP,
        UDP = IPPROTO_UDP,
    };

    class Address {
    public:
        enum {
            ANY,
        };

        Family family() const {
            return Family(addr()->sa_family);
        }

        virtual const sockaddr *addr() const = 0;

        virtual socklen_t len() const = 0;
    };

    class AnyAddress : public Address {
    public:
        AnyAddress() {
            std::memset(&m_u, 0, sizeof(m_u));
            m_size = sizeof(m_u);
        }

        AnyAddress(const sockaddr *addr, socklen_t len) {
            std::memcpy(&m_u, addr, len);
            m_size = len;
        }

        const sockaddr *addr() const final {
            return (struct sockaddr *) (&m_u);
        }

        socklen_t len() const final {
            return m_size;
        }

        void set_address(const sockaddr *addr, socklen_t len) {
            std::memcpy(&m_u, addr, len);
            m_size = len;
        }

        sockaddr *raddr() {
            return (struct sockaddr *) (&m_u);
        }

        socklen_t &rlen() {
            return m_size;
        }

    private:
        struct {
            char buffer[256];
        } m_u;
        socklen_t m_size;
    };

    class IPv4 : public Address {
    public:
        IPv4(const std::string &ip, int port)
                : m_addr({0}) {
            std::memset(&m_addr, 0, sizeof(m_addr));
            m_addr.sin_family = AF_INET;
            if (inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr) <= 0) {
                throw SocketException(concat("inet_pton failed. IP \"", ip, "\" not valid."));
            }
            m_addr.sin_port = htons(port);
        }

        IPv4(decltype(Address::ANY), int port)
                : m_addr({0}) {
            std::memset(&m_addr, 0, sizeof(m_addr));
            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            m_addr.sin_port = htons(port);
        }

        const sockaddr *addr() const final {
            return (struct sockaddr *) (&m_addr);
        }

        socklen_t len() const final {
            return socklen_t(sizeof(m_addr));
        }

        operator AnyAddress() const {
            return AnyAddress(addr(), len());
        }

    private:
        sockaddr_in m_addr;
    };

    class IPv6 : public Address {
    public:
        IPv6(const std::string &ip, int port)
                : m_addr({0}) {
            std::memset(&m_addr, 0, sizeof(m_addr));
            m_addr.sin6_family = AF_INET6;
            if (inet_pton(AF_INET6, ip.c_str(), &m_addr.sin6_addr) <= 0) {
                throw SocketException(concat("inet_pton failed. IP \"", ip, "\" not valid."));
            }
            m_addr.sin6_port = htons(port);
        }

        IPv6(decltype(Address::ANY), int port)
                : m_addr({0}) {
            std::memset(&m_addr, 0, sizeof(m_addr));
            m_addr.sin6_family = AF_INET6;
            m_addr.sin6_addr = in6addr_any;
            m_addr.sin6_port = htons(port);
        }

        const sockaddr *addr() const final {
            return (struct sockaddr *) (&m_addr);
        }

        socklen_t len() const final {
            return socklen_t(sizeof(m_addr));
        }

        operator AnyAddress() const {
            return AnyAddress(addr(), len());
        }

    private:
        sockaddr_in6 m_addr;
    };

    inline AnyAddress make_address(const std::string &ip, int port) {
        static const std::regex ipv6(
                R"(^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|)"
                R"(((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f)"
                R"(]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4])"
                R"(\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f)"
                R"(]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(()"
                R"([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\)"
                R"(d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[)"
                R"(0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-)"
                R"(5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:)"
                R"([0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)"
                R"()){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d)"
                R"(|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$)");
        static const std::regex ipv4(
                R"(^(25[0-5]|2[0-4]\d|[0-1]?\d?\d)(\.(25[0-5]|2[0-4]\d|[0-1]?\d?\d)){3}$)");
        if (std::regex_match(ip, ipv4)) {
            return IPv4(ip, port);
        }
        if (std::regex_match(ip, ipv6)) {
            return IPv6(ip, port);
        }
        throw SocketException(concat("Can not tell is IPv4 or IPv6 address: \"", ip, "\""));
    }

    /**
     * wrapper of web socket
     */
    class Socket {
    public:
        using self = Socket;

        enum class Type : int {
            STREAM = SOCK_STREAM,
            DGRAM = SOCK_DGRAM,
        };

        enum Flag {
            WAIT_ALL = MSG_WAITALL,
        };

        Socket(const Socket &) = delete;

        Socket &operator=(const Socket &) = delete;

        Socket(Socket &&other) OHM_NOEXCEPT {
            std::swap(m_socket, other.m_socket);
        }

        Socket &operator=(Socket &&other) OHM_NOEXCEPT {
            this->close();
            std::swap(m_socket, other.m_socket);
            return *this;
        }

        Socket(Protocol protocol, Family domain)
                : self(domain, Protocol2Type(protocol), protocol) {}

        Socket(Type type, Family domain)
                : self(domain, type, Type2Protocol(type)) {}

        ~Socket() {
            close();
        }

        void close() {
            if (m_socket) {
                ::CLOSE_SOCKET(m_socket);
                m_socket = 0;
            }
        }

        void bind(const Address &address) {
            if (::bind(m_socket, address.addr(), address.len()) == -1) {
                throw SocketException(GetLastSocketError("bind socket failed: "));
            }
            m_address = AnyAddress(address.addr(), address.len());
        }

        void listen(int backlog = 16) {
            if (::listen(m_socket, backlog) == -1) {
                throw SocketException(GetLastSocketError("listen socket failed: "));
            }
        }

        void connect(const Address &address) {
            if (::connect(m_socket, address.addr(), address.len()) < 0) {
                throw SocketException(GetLastSocketError("connect socket failed: "));
            }
            m_address = AnyAddress(address.addr(), address.len());
        }

        Socket accept() const {
            AnyAddress addr;
            auto connected = ::accept(m_socket, addr.raddr(), &addr.rlen());
            if (connected == INVALID_SOCKET) {
                throw SocketException(GetLastSocketError("accept socket failed: "));
            }
            Socket tmp(connected);
            tmp.m_address = addr;
            return std::move(tmp);
        }

        const Address &address() const {
            return m_address;
        }

        bool eof() const {
            return _::SocketEOF(m_socket);
        }

        int recv(void *buf, int len, int flags = 0) const {
            auto size = ::recv(m_socket, reinterpret_cast<char *>(buf), len, flags);
            if (size < 0) {
                throw SocketException(GetLastSocketError("recv socket failed: "));
            }
            return size;
        }

        int send(const void *buf, int len, int flags = 0) const {
            auto size = ::send(m_socket, reinterpret_cast<const char *>(buf), len, flags);
            if (size < 0) {
                throw SocketException(GetLastSocketError("send socket failed: "));
            }
            return size;
        }

        static Type Protocol2Type(Protocol protocol) {
            switch (protocol) {
                case Protocol::TCP:
                    return Type::STREAM;
                case Protocol::UDP:
                    return Type::DGRAM;
            }
            throw SocketException(concat("Unknown protocol: ", int(protocol)));
        }

        static Protocol Type2Protocol(Type type) {
            switch (type) {
                case Type::STREAM:
                    return Protocol::TCP;
                case Type::DGRAM:
                    return Protocol::UDP;
            }
            throw SocketException(concat("Unknown type: ", int(type)));
        }

    private:
        SocketEnv m_env;
        SOCKET_T m_socket = 0;
        AnyAddress m_address;

        Socket(Family domain, Type type, Protocol protocol) {
            m_socket = socket(int(domain), int(type), int(protocol));
            if (m_socket == INVALID_SOCKET) {
                throw SocketException(GetLastSocketError("create socket failed: "));
            }
        }

        explicit Socket(SOCKET_T socket)
                : m_socket(socket) {
            if (m_socket == INVALID_SOCKET) {
                throw SocketException(concat("Go invalid socket."));
            }
        }
    };

    class Connection {
    public:
        explicit Connection(std::shared_ptr<Socket> socket)
                : m_socket(std::move(socket)) {}

        bool eof() const {
            return m_socket->eof();
        }

        int recv(void *buf, int len, int flags = 0) const {
            return m_socket->recv(buf, len, flags);
        }

        int send(const void *buf, int len, int flags = 0) const {
            return m_socket->send(buf, len, flags);
        }

        const Address &address() {
            return m_socket->address();
        }

        void close() {
            m_socket->close();
        }

    private:
        std::shared_ptr<Socket> m_socket;
    };

    class Server {
    public:
        Server(Protocol protocol, const Address &address, int backlog = 16) {
            m_socket = std::make_shared<Socket>(protocol, address.family());
            m_socket->bind(address);
            m_socket->listen(backlog);
        }

        Connection accept() {
            return Connection(std::make_shared<Socket>(m_socket->accept()));
        }

        void close() {
            m_socket->close();
        }

    private:
        std::shared_ptr<Socket> m_socket;
    };

    class Client {
    public:
        static Connection Connect(Protocol protocol, const Address &address) {
            auto socket = std::make_shared<Socket>(protocol, address.family());
            socket->connect(address);
            return Connection(socket);
        }
    };
}

#endif //OMEGA_SOCKET_H
