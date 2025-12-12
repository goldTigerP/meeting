
#include <chrono>
#include <cstdio>

#include "core/network/discovery_service.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        return 0;
    }
    printf("hello world\n");

    std::string name{argv[1]};
    std::string ip{"test"};
    meeting::network::DiscoveryService s{meeting::network::NodeInfo{name, ip, 42}};
    if (s.start()) {
        printf("start\n");
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    return 0;
}