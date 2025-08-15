#include <iostream>
#include <sys/socket.h>   // socket(), bind(), recvfrom(), sendto()
#include <netinet/in.h>   // sockaddr_in structure
#include <unistd.h>       // close()
#include <cstring>        // memset, etc.

int main() {
    // Create a UDP socket (AF_INET = IPv4, SOCK_DGRAM = UDP)
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Prepare address structures for server and client
    sockaddr_in addr{}, client_addr{};
    socklen_t len = sizeof(client_addr);

    // Configure the server address to listen on all interfaces at port 9001
    addr.sin_family = AF_INET;           // IPv4
    addr.sin_port = htons(9001);         // Port 9001 (network byte order)
    addr.sin_addr.s_addr = INADDR_ANY;   // Accept connections from any IP

    // Bind the socket to the specified IP and port
    bind(sock, (sockaddr*)&addr, sizeof(addr));

    std::cout << "UDP Server listening on port 9001..." << std::endl;

    // Buffer to store incoming data
    char buffer[1024];

    // Continuously receive messages and echo them back
    while (true) {
        // Receive message from a client
        ssize_t bytes = recvfrom(sock, buffer, sizeof(buffer), 0,
                                 (sockaddr*)&client_addr, &len);

        // Echo the message back to the client
        sendto(sock, buffer, bytes, 0, (sockaddr*)&client_addr, len);
    }

    // Close the socket when done
    close(sock);
    return 0;
}