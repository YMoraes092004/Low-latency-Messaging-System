#include <iostream>
#include <chrono>
#include <string>
#include <cerrno>
#include <sys/socket.h>   // socket(), sendto(), recvfrom(), setsockopt()
#include <netinet/in.h>   // sockaddr_in, htons, INADDR_*
#include <arpa/inet.h>    // inet_pton()
#include <unistd.h>       // close()
#include <cstring>        // memset
#include "message.hpp"    // Message{id, timestamp, payload} with to_string()/from_string()

using namespace std::chrono;

int main() {
    // --- Create UDP socket ---
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    // --- Optional: recv timeout so we can retry on loss ---
    timeval tv{};
    tv.tv_sec  = 0;
    tv.tv_usec = 50'000; // 50 ms
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt(SO_RCVTIMEO)");
        // not fatal; continue
    }

    // --- Server address ---
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(9001);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    // --- Buffers & counters ---
    const int N = 10'000;
    char buffer[2048];
    int losses = 0;

    // --- Send/recv loop ---
    for (int i = 0; i < N; ++i) {
        // 1) Build message with monotonic send timestamp (µs)
        long long send_ts_us = duration_cast<microseconds>(
                                   steady_clock::now().time_since_epoch()
                               ).count();
        Message msg{i, send_ts_us, "PING"};
        std::string out = msg.to_string(); // e.g., "id,timestamp,payload"

        // 2) Send datagram to server (one shot)
        ssize_t sent = sendto(sock, out.data(), out.size(), 0,
                              reinterpret_cast<const sockaddr*>(&server_addr),
                              sizeof(server_addr));
        if (sent < 0) { perror("sendto"); close(sock); return 1; }

        // 3) Receive echo (with simple retry on timeout)
        bool ok = false;
        for (int attempt = 0; attempt < 2 && !ok; ++attempt) {
            sockaddr_in from_addr{};
            socklen_t   from_len = sizeof(from_addr);
            ssize_t     bytes_in = recvfrom(sock, buffer, sizeof(buffer), 0,
                                            reinterpret_cast<sockaddr*>(&from_addr),
                                            &from_len);
            if (bytes_in > 0) {
                // Compute RTT in microseconds
                long long recv_ts_us = duration_cast<microseconds>(
                                           steady_clock::now().time_since_epoch()
                                       ).count();
                long long rtt_us = recv_ts_us - send_ts_us;

                // (Optional) Parse echoed message for sanity
                // std::string line(buffer, buffer + bytes_in);
                // Message echoed = Message::from_string(line);
                // if (echoed.id != i) { /* handle mismatch if you want */ }

                if ((i + 1) % 1000 == 0) {
                    std::cout << "[UDP] Sent " << (i + 1) << "/" << N
                              << "  Last RTT(µs): " << rtt_us << '\n';
                }
                ok = true;
            } else {
                // Timeout or error
                if (bytes_in < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    // one retry: resend and loop once more to recv
                    if (attempt == 0) {
                        ssize_t rsent = sendto(sock, out.data(), out.size(), 0,
                                               reinterpret_cast<const sockaddr*>(&server_addr),
                                               sizeof(server_addr));
                        if (rsent < 0) { perror("sendto(retry)"); close(sock); return 1; }
                    }
                } else {
                    perror("recvfrom");
                    close(sock);
                    return 1;
                }
            }
        }

        if (!ok) {
            ++losses; // no echo received after retry
        }
    }

    if (losses > 0) {
        std::cout << "[UDP] Lost " << losses << " / " << N << " messages ("
                  << (100.0 * losses / N) << "%)\n";
    }

    close(sock);
    return 0;
}