#include <iostream>
#include <sys/socket.h>   // socket(), bind(), listen(), accept(), read(), write()
#include <netinet/in.h>   // sockaddr_in
#include <unistd.h>       // close()
#include <cstring>        // memset

int main() {
    // 1) Create TCP listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2) Bind to 0.0.0.0:9000
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        std::perror("bind");
        return 1;
    }

    // 3) Listen (one client is fine for now)
    if (listen(server_fd, 1) != 0) {
        std::perror("listen");
        return 1;
    }
    std::cout << "TCP Server listening on port 9000...\n";

    // 4) Accept a client
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        std::perror("accept");
        close(server_fd);
        return 1;
    }

    // 5) Echo loop: read bytes and write them back
    //    We don’t parse here—just echo, including the newline delimiter the client sends.
    char buffer[2048];
    ssize_t n;
    while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t sent = send(client_fd, buffer + off, n - off, 0);
            if (sent <= 0) { std::perror("send"); close(client_fd); close(server_fd); return 1; }
            off += sent;
        }
    }

    // 6) Cleanup
    close(client_fd);
    close(server_fd);
    return 0;
}