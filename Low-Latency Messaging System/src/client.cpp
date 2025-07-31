#include <iostream>
#include <chrono>
#include "message.hpp"

int main() {
    // Get current timestamp in milliseconds
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    // Create a message
    Message m{1, timestamp, "PING"};

    // Convert to string to send over network
    std::string serialized = m.to_string();

    // For now, just print it — next week you'll send this via a socket!
    std::cout << "Client is sending: " << serialized << std::endl;

    return 0;
};
