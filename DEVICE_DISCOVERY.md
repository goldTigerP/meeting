# 设备发现功能使用说明

## 功能概述

本项目实现了基于Qt的局域网设备发现功能，支持自动发现局域网内的其他设备，并获取它们的通信组播地址。

## 核心特性

### 1. 自动设备发现
- 向预设组播地址 `239.255.43.21:45454` 发送发现请求
- 自动接收并处理其他设备的回复
- 解析回复消息中的真正通信组播地址信息

### 2. 心跳机制
- 每1秒自动发送心跳包，保持设备在线状态
- 自动检测设备超时（默认5秒）
- 实时更新在线设备列表

### 3. 通信地址获取
- 从设备回复中解析真正的通信组播地址
- 默认通信地址：`239.255.43.22:45455`
- 支持动态配置不同的通信地址

## 技术架构

### 网络层设计

```
预设发现层 (UDP组播)
├── 地址: 239.255.43.21:45454
├── 功能: 设备发现、心跳、离线通知
└── 消息类型:
    ├── DISCOVERY_REQUEST (发现请求)
    ├── DISCOVERY_RESPONSE (发现回复) 
    ├── HEARTBEAT (心跳包)
    └── DEVICE_OFFLINE (离线通知)

通信层 (从发现回复中获取)
├── 地址: 239.255.43.22:45455 (默认)
├── 功能: 实际的消息通信、文件传输等
└── 可根据设备配置动态变化
```

### 消息协议格式

```json
{
  "type": 1,                    // 消息类型
  "deviceId": "uuid-string",    // 设备唯一ID
  "timestamp": 1638360000000,   // 时间戳
  "data": {                     // 消息数据
    "deviceName": "设备名称",
    "localAddress": "192.168.1.100",
    "communicationMulticastAddress": "239.255.43.22",
    "communicationPort": 45455
  }
}
```

## 核心类说明

### DiscoveryService 类

**主要功能：**
- 设备发现和心跳管理
- 网络消息处理
- 在线设备列表维护

**关键方法：**
```cpp
bool startDiscovery(const QString& deviceName, const QString& deviceId = QString());
void stopDiscovery();
QList<DeviceInfo> getOnlineDevices() const;
void setHeartbeatInterval(int intervalMs);
void setDeviceTimeout(int timeoutMs);
```

**信号：**
```cpp
void deviceDiscovered(const QString& deviceId, const QString& deviceName, const QHostAddress& address);
void deviceLeft(const QString& deviceId);
void communicationAddressReceived(const QString& deviceId, const QHostAddress& multicastAddress, quint16 port);
void errorOccurred(const QString& error);
```

## 使用示例

### 1. 基本使用

```cpp
#include "service/discovery.h"

// 创建发现服务
DiscoveryService* discoveryService = new DiscoveryService(this);

// 连接信号
connect(discoveryService, &DiscoveryService::deviceDiscovered,
        this, &MyClass::onDeviceFound);
connect(discoveryService, &DiscoveryService::communicationAddressReceived,
        this, &MyClass::onCommunicationAddressReceived);

// 启动发现
discoveryService->startDiscovery("我的设备名称");
```

### 2. 处理发现的设备

```cpp
void MyClass::onDeviceFound(const QString& deviceId, const QString& deviceName, const QHostAddress& address)
{
    qDebug() << "发现设备:" << deviceName << "IP:" << address.toString();
    
    // 获取设备详细信息
    DiscoveryService::DeviceInfo info = discoveryService->getDeviceInfo(deviceId);
    
    // 现在可以使用 info.communicationMulticastAddress 进行实际通信
}

void MyClass::onCommunicationAddressReceived(const QString& deviceId, 
                                            const QHostAddress& multicastAddress, 
                                            quint16 port)
{
    qDebug() << "设备" << deviceId << "的通信地址:" << multicastAddress.toString() << ":" << port;
    
    // 可以基于这个地址建立实际的通信连接
    setupCommunicationChannel(multicastAddress, port);
}
```

## 配置参数

### 网络配置
- **预设组播地址**: `239.255.43.21:45454`
- **通信组播地址**: `239.255.43.22:45455` (可配置)
- **心跳间隔**: 1000ms (可配置)
- **设备超时**: 5000ms (可配置)

### 修改配置

```cpp
// 修改心跳间隔为2秒
discoveryService->setHeartbeatInterval(2000);

// 修改超时时间为10秒
discoveryService->setDeviceTimeout(10000);
```

## 测试方法

1. 编译项目：
```bash
cd build
cmake ..
make
```

2. 运行测试程序：
```bash
./bin/meeting
```

3. 选择"设备发现功能测试"

4. 输入设备名称并点击"启动发现"

5. 在多台设备上同时运行，观察设备发现过程

## 扩展建议

### 1. 安全增强
- 添加设备认证机制
- 实现消息加密传输
- 添加访问控制列表

### 2. 性能优化  
- 实现智能心跳频率调整
- 添加网络状态检测
- 优化大量设备场景下的性能

### 3. 功能扩展
- 支持设备分组管理
- 添加设备能力协商
- 实现设备状态同步

## 注意事项

1. **防火墙设置**: 确保UDP端口45454开放
2. **网络环境**: 需要支持组播的网络环境
3. **权限要求**: 可能需要管理员权限来绑定端口
4. **资源清理**: 应用退出时记得调用`stopDiscovery()`

## 下一步计划

基于设备发现功能，可以继续实现：
1. 文本消息传输服务
2. 文件传输服务  
3. 音视频通话功能
4. 群组管理功能