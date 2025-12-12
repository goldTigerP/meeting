#include "discovery_service.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define close closesocket
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "util/common_define.h"

namespace meeting {
namespace network {

DiscoveryService::DiscoveryService(const NodeInfo &node)
    : local_node_(node),
      multicast_address_("239.255.255.250"),
      multicast_port_(12345),
      heartbeat_interval_(1000),
      node_timeout_(5000),
      multicast_socket_(-1),
      running_(false) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

DiscoveryService::~DiscoveryService() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool DiscoveryService::start() {
    if (running_) {
        return true;
    }

    if (!initializeSocket()) {
        LOG_ERROR << "Failed to initialize multicast socket";
        return false;
    }

    running_ = true;

    // 启动线程
    sender_thread_   = std::thread(&DiscoveryService::senderLoop, this);
    receiver_thread_ = std::thread(&DiscoveryService::receiverLoop, this);
    cleanup_thread_  = std::thread(&DiscoveryService::cleanupLoop, this);

    LOG_INFO << "Discovery service started for node: " << local_node_.id << " (" << local_node_.ip
             << ":" << local_node_.port << ")";

    return true;
}

void DiscoveryService::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    // 等待线程结束
    if (sender_thread_.joinable()) {
        sender_thread_.join();
    }
    if (receiver_thread_.joinable()) {
        receiver_thread_.join();
    }
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }

    cleanupSocket();

    std::cout << "Discovery service stopped" << std::endl;
}

bool DiscoveryService::initializeSocket() {
    // 创建UDP套接字
    multicast_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicast_socket_ < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    // 设置套接字选项：允许地址重用
    int reuse = 1;
    if (setsockopt(multicast_socket_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char *>(&reuse), sizeof(reuse)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        close(multicast_socket_);
        return false;
    }

#ifdef SO_REUSEPORT
    if (setsockopt(multicast_socket_, SOL_SOCKET, SO_REUSEPORT,
                   reinterpret_cast<const char *>(&reuse), sizeof(reuse)) < 0) {
        // SO_REUSEPORT 在某些系统上可能不支持，这不是致命错误
    }
#endif

    // 绑定到多播端口
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family      = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port        = htons(multicast_port_);

    if (bind(multicast_socket_, reinterpret_cast<struct sockaddr *>(&bind_addr),
             sizeof(bind_addr)) < 0) {
        std::cerr << "Failed to bind socket to port " << multicast_port_ << std::endl;
        close(multicast_socket_);
        return false;
    }

    // 加入多播组
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_address_.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(multicast_socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0) {
        std::cerr << "Failed to join multicast group" << std::endl;
        close(multicast_socket_);
        return false;
    }

    // 设置多播TTL
    int ttl = 3;
    if (setsockopt(multicast_socket_, IPPROTO_IP, IP_MULTICAST_TTL,
                   reinterpret_cast<const char *>(&ttl), sizeof(ttl)) < 0) {
        std::cerr << "Failed to set multicast TTL" << std::endl;
    }

    int loopback = 1;
    if (setsockopt(multicast_socket_, IPPROTO_IP, IP_MULTICAST_LOOP,
                   reinterpret_cast<const char *>(&loopback), sizeof(loopback)) < 0) {
        std::cerr << "Failed to disable multicast loopback" << std::endl;
    }

    return true;
}

void DiscoveryService::cleanupSocket() {
    if (multicast_socket_ >= 0) {
        // 离开多播组
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(multicast_address_.c_str());
        mreq.imr_interface.s_addr = INADDR_ANY;

        setsockopt(multicast_socket_, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                   reinterpret_cast<const char *>(&mreq), sizeof(mreq));

        close(multicast_socket_);
        multicast_socket_ = -1;
    }
}

void DiscoveryService::senderLoop() {
    struct sockaddr_in multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family      = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr(multicast_address_.c_str());
    multicast_addr.sin_port        = htons(multicast_port_);

    std::string message = createAnnounceMessage();
    while (running_) {
        ssize_t sent =
            sendto(multicast_socket_, message.c_str(), message.length(), 0,
                   reinterpret_cast<struct sockaddr *>(&multicast_addr), sizeof(multicast_addr));

        if (sent < 0) {
            std::cerr << "Failed to send multicast message" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval_));
    }
}

void DiscoveryService::receiverLoop() {
    char buffer[1024];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    while (running_) {
        ssize_t received = recvfrom(multicast_socket_, buffer, sizeof(buffer) - 1, 0,
                                    reinterpret_cast<struct sockaddr *>(&sender_addr), &sender_len);

        if (received > 0) {
            buffer[received] = '\0';
            std::string message(buffer);

            NodeInfo node;
            if (parseReceivedMessage(message, node)) {
                if (node.id != local_node_.id) {
                    handleNodeDiscovered(node);
                }
            }
        }
    }
}

void DiscoveryService::cleanupLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval_));
        removeExpiredNodes();
    }
}

std::string DiscoveryService::createAnnounceMessage() {
    std::stringstream ss;
    ss << "MEETING_DISCOVERY|" << local_node_.id << "|" << local_node_.ip << "|"
       << local_node_.port;
    return ss.str();
}

bool DiscoveryService::parseReceivedMessage(const std::string &message, NodeInfo &node) {
    if (message.substr(0, 17) != "MEETING_DISCOVERY") {
        return false;
    }

    std::stringstream ss(message);
    std::string token;
    std::vector<std::string> parts;

    while (std::getline(ss, token, '|')) {
        parts.push_back(token);
    }

    if (parts.size() < 4) {
        return false;
    }

    try {
        node.id   = parts[1];
        node.ip   = parts[2];
        node.port = static_cast<uint16_t>(std::stoul(parts[3]));
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

void DiscoveryService::handleNodeDiscovered(const NodeInfo &node) {
    std::lock_guard<std::mutex> lock(nodes_mutex_);
    auto now = GetCurrentTimeStamp();
    auto it  = discovered_nodes_.find(node.id);
    if (it == discovered_nodes_.end()) {
        discovered_nodes_[node.id] = {node, now};
        notifyCallback(DiscoveryEvent::NODE_JOINED, node);
    } else {
        it->second.second = now;
        if (it->second.first.CompareAndUpdate(node)) {
            notifyCallback(DiscoveryEvent::NODE_UPDATED, node);
        }
    }
}

void DiscoveryService::removeExpiredNodes() {
    std::lock_guard<std::mutex> lock(nodes_mutex_);
    auto now        = GetCurrentTimeStamp();
    auto timeout_ns = node_timeout_ * 1000 * 1000;

    auto it = discovered_nodes_.begin();
    while (it != discovered_nodes_.end()) {
        auto duration = now - it->second.second;

        if (duration > timeout_ns) {
            NodeInfo expired_node = it->second.first;
            notifyCallback(DiscoveryEvent::NODE_LEFT, expired_node);
            it = discovered_nodes_.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<NodeInfo> DiscoveryService::getDiscoveredNodes() const {
    std::lock_guard<std::mutex> lock(nodes_mutex_);
    std::vector<NodeInfo> nodes;
    nodes.reserve(discovered_nodes_.size());

    for (const auto &pair : discovered_nodes_) {
        nodes.push_back(pair.second.first);
    }

    return nodes;
}

void DiscoveryService::notifyCallback(DiscoveryEvent event, const NodeInfo &node) {
    if (discovery_callback_ != nullptr) {
        discovery_callback_(event, node);
    } else {
        std::cout << (int)event << "\t id:" << node.id << '\n';
    }
}

}  // namespace network
}  // namespace meeting