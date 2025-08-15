#include <iostream>
#include <sys/socket.h>   // socket(), connect(), send(), recv()
#include <netinet/in.h>   // sockaddr_in
#include <arpa/inet.h>    // inet_pton()
#include <unistd.h>       // close()
#include <cstring>        // memset
#include <chrono>
#include <string>
#include "message.hpp"    // Message{id, timestamp, payload} with to_string()/from_string()

int main() {
    // 1) Create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 2) Server address (localhost:9000)
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    // 3) Connect to server
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0) {
        std::perror("connect");
        return 1;
    }

    // Reusable receive buffer
    char buffer[2048];
    std::string recv_buf;   // std::string, persists across iterations

    // 4) Send N messages and measure round-trip latency
    const int N = 10000;

    for (int i = 0; i < N; ++i) {
        // 4a) Create timestamp (µs)
        auto now = std::chrono::steady_clock::now();
        long long ts = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

        // 4b) Build serialized message and add newline delimiter
        Message m{i, ts, "PING"};
        std::string out = m.to_string();
        out.push_back('\n');  // delimiter for TCP framing

        // 4c) Send all bytes
        const char* data = out.c_str();
        size_t to_send = out.size();
        while (to_send > 0) {
            ssize_t sent = send(sock, data, to_send, 0);
            if (sent <= 0) { std::perror("send"); close(sock); return 1; }
            data    += sent;
            to_send -= sent;
        }

        // 4d) Receive until newline (full echoed message)

        while (recv_buf.find('\n') == std::string::npos) {
            ssize_t n = recv(sock, buffer, sizeof(buffer), 0);
            if (n <= 0){ 
                std::perror("recv"); 
                close(sock); 
                return 1; 
            }
            recv_buf.append(buffer, static_cast<size_t>(n));
        }

        // Extract exactly one line for THIS iteration
        size_t nl_pos = recv_buf.find('\n');
        std::string line = recv_buf.substr(0, nl_pos);  // without '\n'
        recv_buf.erase(0, nl_pos + 1);                  // keep leftovers for next loop
        
        // 4e) Compute latency based on original send timestamp
        //     (We embedded ts in the payload, but for now we just reuse ts we captured)
        long long recv_ts = std::chrono::duration_cast<std::chrono::microseconds>(
                                std::chrono::steady_clock::now().time_since_epoch()
                            ).count();
        long long rtt_µs = recv_ts - ts;

        // Optional: parse the echoed string back to a Message (drop trailing '\n')
        Message echoed = Message::from_string(line);

        // Log occasionally so output isn’t massive
        if ((i + 1) % 1000 == 0) {
            std::cout << "[TCP] Sent " << (i + 1) << "/" << N
                      << "  Last RTT(µs): " << rtt_µs << '\n';
        }
    }

    // 5) Close after the loop
    close(sock);
    return 0;
}