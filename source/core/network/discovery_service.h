#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace meeting {
namespace network {

struct NodeInfo {
    std::string id;
    std::string ip;
    uint16_t port;

    NodeInfo() = default;
    NodeInfo(const NodeInfo &node) : id(node.id), ip(node.ip), port(node.port) {}
    NodeInfo(const std::string &id, const std::string &ip, uint16_t p) : id(id), ip(ip), port(p) {}

    bool CompareAndUpdate(const NodeInfo &other) {
        if (id == other.id && ip == other.ip && port == other.port) {
            return false;
        }
        id   = other.id;
        ip   = other.ip;
        port = other.port;
        return true;
    }
};

enum class DiscoveryEvent : uint8_t {
    NODE_JOINED,  // 新节点加入
    NODE_LEFT,    // 节点离开
    NODE_UPDATED  // 节点信息更新
};

using DiscoveryCallback = std::function<void(DiscoveryEvent event, const NodeInfo &node)>;

class DiscoveryService {
public:
    DiscoveryService(const NodeInfo &node);

    ~DiscoveryService();

    bool start();

    void stop();

    bool isRunning() const {
        return running_;
    }

    void setDiscoveryCallback(DiscoveryCallback &&callback) {
        discovery_callback_ = std::move(callback);
    }

    std::vector<NodeInfo> getDiscoveredNodes() const;

    const NodeInfo &getLocalNodeInfo() const {
        return local_node_;
    }

    void setHeartbeatInterval(int seconds) {
        heartbeat_interval_ = seconds;
    }

    void setNodeTimeout(int seconds) {
        node_timeout_ = seconds;
    }

private:
    // 配置参数
    std::string local_ip_;
    uint16_t local_port_;
    std::string multicast_address_;
    uint16_t multicast_port_;
    int heartbeat_interval_;  // 心跳间隔（秒）
    int64_t node_timeout_;    // 节点超时时间（秒）

    // 本地节点信息
    NodeInfo local_node_;

    // 发现的远程节点
    mutable std::mutex nodes_mutex_;
    std::unordered_map<std::string, std::pair<NodeInfo, int64_t>> discovered_nodes_;

    // 网络相关
    int multicast_socket_;

    // 线程控制
    std::atomic<bool> running_;
    std::thread sender_thread_;
    std::thread receiver_thread_;
    std::thread cleanup_thread_;

    // 回调函数
    DiscoveryCallback discovery_callback_;

    // 内部方法
    bool initializeSocket();
    void cleanupSocket();

    // 线程函数
    void senderLoop();
    void receiverLoop();
    void cleanupLoop();

    // 消息处理
    std::string createAnnounceMessage();
    bool parseReceivedMessage(const std::string &message, NodeInfo &node);
    void handleNodeDiscovered(const NodeInfo &node);
    void removeExpiredNodes();

    // 工具函数
    void notifyCallback(DiscoveryEvent event, const NodeInfo &node);
};

}  // namespace network
}  // namespace meeting