#include "discovery_service.h"
#include "../util/network_util.h"
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QUuid>

// 预设的组播地址和端口
const QHostAddress DiscoveryService::PRESET_MULTICAST_ADDRESS =
    QHostAddress("239.255.43.21");
const quint16 DiscoveryService::PRESET_MULTICAST_PORT = 45454;

DiscoveryService::DiscoveryService(QObject *parent)
    : QObject(parent), m_socket(nullptr), m_heartbeatTimer(new QTimer(this)),
      m_timeoutTimer(new QTimer(this)),
      m_communicationPort(45455), // 默认通信端口
      m_heartbeatInterval(1000),  // 1秒心跳间隔
      m_deviceTimeout(5000),      // 5秒超时
      m_isRunning(false) {
  // 设置真正用于通信的组播地址（可以根据需要配置）
  m_communicationMulticastAddress = QHostAddress("239.255.43.22");

  // 连接定时器信号
  connect(m_heartbeatTimer, &QTimer::timeout, this,
          &DiscoveryService::sendHeartbeat);
  connect(m_timeoutTimer, &QTimer::timeout, this,
          &DiscoveryService::checkDeviceTimeout);
}

DiscoveryService::~DiscoveryService() { stopDiscovery(); }

bool DiscoveryService::startDiscovery(const QString &deviceName,
                                      const QString &deviceId) {
  if (m_isRunning) {
    qWarning() << "Discovery service is already running";
    return false;
  }

  m_deviceId = deviceName;
  m_localAddress = getLocalAddress();

  if (m_localAddress.isNull()) {
    emit errorOccurred("Failed to get local IP address");
    return false;
  }

  // 初始化网络
  if (!initializeNetwork()) {
    emit errorOccurred("Failed to initialize network");
    return false;
  }

  qDebug() << "Discovery service started for device:" << m_deviceName
           << "ID:" << m_deviceId;
  qDebug() << "Local address:" << m_localAddress.toString();
  qDebug() << "Preset multicast address:" << PRESET_MULTICAST_ADDRESS.toString()
           << ":" << PRESET_MULTICAST_PORT;
  qDebug() << "Communication multicast address:"
           << m_communicationMulticastAddress.toString() << ":"
           << m_communicationPort;

  // 发送初始发现请求
  sendDiscoveryRequest();

  // 启动定时器
  m_heartbeatTimer->start(m_heartbeatInterval);
  m_timeoutTimer->start(1000); // 每秒检查一次超时

  m_isRunning = true;
  return true;
}

void DiscoveryService::stopDiscovery() {
  if (!m_isRunning) {
    return;
  }

  // 发送离线消息
  QJsonObject offlineData;
  offlineData["reason"] = "normal_shutdown";
  QJsonObject offlineMessage = createMessage(DEVICE_OFFLINE, offlineData);
  QByteArray data =
      QJsonDocument(offlineMessage).toJson(QJsonDocument::Compact);

  // 向预设组播地址发送离线消息
  m_socket->writeDatagram(data, PRESET_MULTICAST_ADDRESS,
                          PRESET_MULTICAST_PORT);

  // 停止定时器
  m_heartbeatTimer->stop();
  m_timeoutTimer->stop();

  // 清理网络资源
  if (m_socket) {
    m_socket->leaveMulticastGroup(PRESET_MULTICAST_ADDRESS);
    m_socket->deleteLater();
    m_socket = nullptr;
  }

  // 清理设备列表
  m_onlineDevices.clear();

  m_isRunning = false;
  qDebug() << "Discovery service stopped";
}

QList<DiscoveryService::DeviceInfo> DiscoveryService::getOnlineDevices() const {
  return m_onlineDevices.values();
}

DiscoveryService::DeviceInfo
DiscoveryService::getDeviceInfo(const QString &deviceId) const {
  return m_onlineDevices.value(deviceId, DeviceInfo());
}

void DiscoveryService::setHeartbeatInterval(int intervalMs) {
  m_heartbeatInterval = intervalMs;
  if (m_isRunning) {
    m_heartbeatTimer->start(m_heartbeatInterval);
  }
}

void DiscoveryService::setDeviceTimeout(int timeoutMs) {
  m_deviceTimeout = timeoutMs;
}

bool DiscoveryService::initializeNetwork() {
  // 创建UDP套接字
  m_socket = new QUdpSocket(this);

  // 连接信号
  connect(m_socket, &QUdpSocket::readyRead, this,
          &DiscoveryService::processPendingDatagrams);

  // 绑定到预设端口
  if (!m_socket->bind(QHostAddress::AnyIPv4, PRESET_MULTICAST_PORT,
                      QUdpSocket::ShareAddress |
                          QUdpSocket::ReuseAddressHint)) {
    qWarning() << "Failed to bind to port" << PRESET_MULTICAST_PORT << ":"
               << m_socket->errorString();
    return false;
  }

  // 加入预设组播组
  if (!m_socket->joinMulticastGroup(PRESET_MULTICAST_ADDRESS)) {
    qWarning() << "Failed to join multicast group"
               << PRESET_MULTICAST_ADDRESS.toString() << ":"
               << m_socket->errorString();
    return false;
  }

  return true;
}

void DiscoveryService::processPendingDatagrams() {
  while (m_socket->hasPendingDatagrams()) {
    QByteArray datagram;
    QHostAddress senderAddress;
    quint16 senderPort;

    datagram.resize(m_socket->pendingDatagramSize());
    m_socket->readDatagram(datagram.data(), datagram.size(), &senderAddress,
                           &senderPort);

    // 解析JSON消息
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(datagram, &error);
    if (error.error != QJsonParseError::NoError) {
      qWarning() << "Failed to parse JSON message:" << error.errorString();
      continue;
    }

    QJsonObject message = doc.object();

    // 检查消息格式
    if (!message.contains("type") || !message.contains("deviceId") ||
        !message.contains("timestamp")) {
      qWarning() << "Invalid message format";
      continue;
    }

    // 忽略自己发送的消息
    QString senderId = message["deviceId"].toString();
    if (senderId == m_deviceId) {
      continue;
    }

    // 根据消息类型处理
    MessageType type = static_cast<MessageType>(message["type"].toInt());
    switch (type) {
    case DISCOVERY_REQUEST:
      handleDiscoveryRequest(message, senderAddress, senderPort);
      break;
    case DISCOVERY_RESPONSE:
      handleDiscoveryResponse(message, senderAddress);
      break;
    case HEARTBEAT:
      handleHeartbeat(message, senderAddress, senderPort);
      break;
    case DEVICE_OFFLINE:
      if (m_onlineDevices.contains(senderId)) {
        emit deviceLeft(senderId);
        m_onlineDevices.remove(senderId);
        qDebug() << "Device went offline:" << senderId;
      }
      break;
    default:
      qWarning() << "Unknown message type:" << type;
      break;
    }
  }
}

void DiscoveryService::sendDiscoveryRequest() {
  QJsonObject requestData;
  requestData["deviceName"] = m_deviceName;
  requestData["localAddress"] = m_localAddress.toString();
  requestData["communicationMulticastAddress"] =
      m_communicationMulticastAddress.toString();
  requestData["communicationPort"] = m_communicationPort;

  QJsonObject requestMessage = createMessage(DISCOVERY_REQUEST, requestData);
  QByteArray data =
      QJsonDocument(requestMessage).toJson(QJsonDocument::Compact);

  // 向预设组播地址发送发现请求
  qint64 sent = m_socket->writeDatagram(data, PRESET_MULTICAST_ADDRESS,
                                        PRESET_MULTICAST_PORT);
  if (sent != data.size()) {
    qWarning() << "Failed to send discovery request";
  }
}

void DiscoveryService::sendDiscoveryResponse(const QHostAddress &targetAddress,
                                             quint16 targetPort) {
  QJsonObject responseData;
  responseData["deviceName"] = m_deviceName;
  responseData["localAddress"] = m_localAddress.toString();
  responseData["communicationMulticastAddress"] =
      m_communicationMulticastAddress.toString();
  responseData["communicationPort"] = m_communicationPort;

  QJsonObject responseMessage = createMessage(DISCOVERY_RESPONSE, responseData);
  QByteArray data =
      QJsonDocument(responseMessage).toJson(QJsonDocument::Compact);

  // 直接回复给请求者
  qint64 sent = m_socket->writeDatagram(data, targetAddress, targetPort);
  if (sent != data.size()) {
    qWarning() << "Failed to send discovery response";
  }
}

void DiscoveryService::sendHeartbeat() {
  if (!m_isRunning) {
    return;
  }

  QJsonObject heartbeatData;
  heartbeatData["deviceName"] = m_deviceName;

  QJsonObject heartbeatMessage = createMessage(HEARTBEAT, heartbeatData);
  QByteArray data =
      QJsonDocument(heartbeatMessage).toJson(QJsonDocument::Compact);

  // 向组播地址发送心跳
  qint64 sent = m_socket->writeDatagram(data, m_communicationMulticastAddress,
                                        m_communicationPort);
  if (sent != data.size()) {
    qWarning() << "Failed to send heartbeat";
  }
}

void DiscoveryService::handleDiscoveryRequest(const QJsonObject &message,
                                              const QHostAddress &senderAddress,
                                              quint16 senderPort) {
  QString senderId = message["deviceId"].toString();

  qDebug() << "Received discovery request from:" << senderId << "at"
           << senderAddress.toString();

  // 发送回复
  sendDiscoveryResponse(senderAddress, senderPort);

  //   // 更新设备信息
  //   if (message.contains("data")) {
  //     QJsonObject data = message["data"].toObject();
  //     updateDeviceInfo(senderId, data, senderAddress);
  //   }
}

void DiscoveryService::handleDiscoveryResponse(
    const QJsonObject &message, const QHostAddress &senderAddress) {
  QString senderId = message["deviceId"].toString();

  qDebug() << "Received discovery response from:" << senderId << "at"
           << senderAddress.toString();

  if (message.contains("data")) {
    QJsonObject data = message["data"].toObject();

    // 解析通信组播地址信息
    if (data.contains("communicationMulticastAddress") &&
        data.contains("communicationPort")) {
      QHostAddress multicastAddr(
          data["communicationMulticastAddress"].toString());
      if (!multicastAddr.isNull()) {
        m_communicationMulticastAddress = multicastAddr;
        m_communicationPort = data["communicationPort"].toInt();
      }
    }
  }
}

void DiscoveryService::handleHeartbeat(const QJsonObject &message,
                                       const QHostAddress &senderAddress,
                                       quint16 senderPort) {
  QString senderId = message["deviceId"].toString();

  // 更新设备信息
  if (message.contains("data")) {
    QJsonObject data = message["data"].toObject();
    updateDeviceInfo(senderId, data, senderAddress, senderPort);
  }
}

void DiscoveryService::updateDeviceInfo(const QString &deviceId,
                                        const QJsonObject &data,
                                        const QHostAddress &senderAddress,
                                        quint16 senderPort) {
  bool isNewDevice = !m_onlineDevices.contains(deviceId);

  DeviceInfo &deviceInfo = m_onlineDevices[deviceId];
  deviceInfo.deviceId = deviceId;
  deviceInfo.address = senderAddress;
  deviceInfo.port = senderPort;
  deviceInfo.lastHeartbeat = QDateTime::currentMSecsSinceEpoch();

  if (isNewDevice) {
    emit deviceDiscovered(deviceId);
    qDebug() << "New device discovered:" << deviceInfo.deviceName << "("
             << deviceId << ")"
             << "at" << senderAddress.toString() << "Communication address:"
             << deviceInfo.communicationMulticastAddress.toString() << ":"
             << deviceInfo.communicationPort;
  }
}

void DiscoveryService::checkDeviceTimeout() {
  qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
  QStringList offlineDevices;

  for (auto it = m_onlineDevices.begin(); it != m_onlineDevices.end(); ++it) {
    if (currentTime - it->lastHeartbeat > m_deviceTimeout) {
      offlineDevices.append(it->deviceId);
    }
  }

  // 移除超时设备
  for (const QString &deviceId : offlineDevices) {
    m_onlineDevices.remove(deviceId);
    emit deviceLeft(deviceId);
    qDebug() << "Device timeout:" << deviceId;
  }
}

QJsonObject DiscoveryService::createMessage(MessageType type,
                                            const QJsonObject &data) const {
  QJsonObject message;
  message["type"] = static_cast<int>(type);
  message["deviceId"] = m_deviceId;
  message["timestamp"] = QDateTime::currentMSecsSinceEpoch();

  if (!data.isEmpty()) {
    message["data"] = data;
  }

  return message;
}

QHostAddress DiscoveryService::getLocalAddress() const {
  return {NetworkUtil::getPrimaryIPAddress()};
}