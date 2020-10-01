//
// Created by kier on 2020/9/28.
//

#include "ohm/socket.h"
#include "ohm/print.h"
#include "ohm/type_name.h"

using namespace ohm;

void server() {
    println("=================== Server =====================");
    try {
        Server server(Protocol::TCP, IPv4(Address::ANY, 2333));
        auto pipe = server.accept();

        char buffer[1024];
        auto n = pipe.recv(buffer, 1024);
        buffer[n] = '\0';
        printf("recv msg from client: %s\n", buffer);
    } catch (const SocketException &e) {
        println(e.what());
    }

}

void client() {
    println("=================== Client =====================");
    try {
        auto pipe = Client::Connect(Protocol::TCP, IPv4("127.0.0.1", 2333));

        char buffer[1024];
        fgets(buffer, 1000, stdin);
        pipe.send(buffer, int(strlen(buffer)));
    } catch (const SocketException &e) {
        println(e.what());
    }
}

int main(int argc, const char *argv[]) {
    if (argc <= 1) {
        println("Usage: ", argv[0], " [server|client]");
        return -1;
    }
    std::string cmd = argv[1];
    if (cmd == "server") {
        server();
    } else if (cmd == "client") {
        client();
    } else {
        println("Usage: ", argv[0], " [server|client]");
        return -1;
    }
}

