#pragma once

#include <QHash>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

/**
 * 设备发现服务类
 * 功能：
 * 1. 向预设组播地址发送发现请求
 * 2. 解析回复消息获取真正的通信组播地址
 * 3. 定时发送心跳包
 * 4. 维护在线设备列表
 */
class DiscoveryService : public QObject {
  Q_OBJECT

public:
  struct DeviceInfo {
    QString deviceId;
    qint64 lastHeartbeat;
    QHostAddress address;
    quint16 port;
  };

  enum MessageType {
    DISCOVERY_REQUEST = 1,  // 发现请求
    DISCOVERY_RESPONSE = 2, // 发现回复
    HEARTBEAT = 3,          // 心跳包
    DEVICE_OFFLINE = 4      // 设备下线
  };

  explicit DiscoveryService(QObject *parent = nullptr);
  virtual ~DiscoveryService();

  bool startDiscovery(const QString &deviceId);

  void stopDiscovery();

  /**
   * 获取当前在线设备列表
   */
  QList<DeviceInfo> getOnlineDevices() const;

  /**
   * 获取指定设备信息
   */
  DeviceInfo getDeviceInfo(const QString &deviceId) const;

  /**
   * 设置心跳间隔（毫秒）
   */
  void setHeartbeatInterval(int intervalMs);

  /**
   * 设置设备超时时间（毫秒）
   */
  void setDeviceTimeout(int timeoutMs);

signals:
  /**
   * 发现新设备
   */
  void deviceDiscovered(const QString &deviceId);

  /**
   * 设备离线
   */
  void deviceLeft(const QString &deviceId);

  /**
   * 接收到通信组播地址信息
   */
  void communicationAddressReceived(const QString &deviceId,
                                    const QHostAddress &multicastAddress,
                                    quint16 port);

  /**
   * 错误信号
   */
  void errorOccurred(const QString &error);

private slots:
  /**
   * 处理接收到的UDP数据
   */
  void processPendingDatagrams();

  /**
   * 发送心跳包
   */
  void sendHeartbeat();

  /**
   * 检查设备超时
   */
  void checkDeviceTimeout();

private:
  /**
   * 初始化网络组件
   */
  bool initializeNetwork();

  /**
   * 发送发现请求
   */
  void sendDiscoveryRequest();

  /**
   * 发送发现回复
   */
  void sendDiscoveryResponse(const QHostAddress &targetAddress,
                             quint16 targetPort);

  /**
   * 处理发现请求
   */
  void handleDiscoveryRequest(const QJsonObject &message,
                              const QHostAddress &senderAddress,
                              quint16 senderPort);

  /**
   * 处理发现回复
   */
  void handleDiscoveryResponse(const QJsonObject &message,
                               const QHostAddress &senderAddress);

  /**
   * 处理心跳包
   */
  void handleHeartbeat(const QJsonObject &message,
                       const QHostAddress &senderAddress);

  /**
   * 创建消息JSON对象
   */
  QJsonObject createMessage(MessageType type,
                            const QJsonObject &data = QJsonObject()) const;

  /**
   * 生成设备ID
   */
  QString generateDeviceId() const;

  /**
   * 获取本机IP地址
   */
  QHostAddress getLocalAddress() const;

  /**
   * 更新设备信息
   */
  void updateDeviceInfo(const QString &deviceId, const QJsonObject &data,
                        const QHostAddress &senderAddress);

private:
  // 网络组件
  QUdpSocket *m_socket; // UDP套接字

  // 定时器
  QTimer *m_heartbeatTimer; // 心跳定时器
  QTimer *m_timeoutTimer;   // 超时检查定时器

  // 设备信息
  QString m_deviceId;          // 本设备ID
  QHostAddress m_localAddress; // 本机地址

  // 组播配置
  static const QHostAddress PRESET_MULTICAST_ADDRESS; // 预设的组播地址
  static const quint16 PRESET_MULTICAST_PORT;         // 预设的组播端口
  QHostAddress m_communicationMulticastAddress; // 真正用于通信的组播地址
  quint16 m_communicationPort;                  // 真正用于通信的端口

  // 在线设备管理
  QHash<QString, DeviceInfo> m_onlineDevices; // 在线设备列表

  // 配置参数
  int m_heartbeatInterval; // 心跳间隔（毫秒）
  int m_deviceTimeout;     // 设备超时时间（毫秒）

  // 状态标志
  bool m_isRunning; // 服务是否运行中
};