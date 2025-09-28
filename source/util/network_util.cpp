#include "network_util.h"
#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

QStringList NetworkUtil::getLocalIPAddresses() {
  QStringList ipAddresses;

  // 获取所有网络接口
  const QList<QNetworkInterface> interfaces =
      QNetworkInterface::allInterfaces();

  for (const QNetworkInterface &interface : interfaces) {
    // 跳过非活跃的接口
    if (!(interface.flags() & QNetworkInterface::IsUp) ||
        !(interface.flags() & QNetworkInterface::IsRunning)) {
      continue;
    }

    // 获取接口的所有IP地址
    const QList<QNetworkAddressEntry> entries = interface.addressEntries();
    for (const QNetworkAddressEntry &entry : entries) {
      const QHostAddress address = entry.ip();

      // 只添加IPv4地址，排除本地回环地址
      if (address.protocol() == QAbstractSocket::IPv4Protocol &&
          !address.isLoopback()) {
        ipAddresses << address.toString();
      }
    }
  }

  return ipAddresses;
}

QString NetworkUtil::getPrimaryIPAddress() {
  const QStringList addresses = getLocalIPAddresses();

  // 优先返回192.168.x.x或10.x.x.x等常见内网地址
  for (const QString &address : addresses) {
    if (address.startsWith("192.168.") || address.startsWith("10.") ||
        address.startsWith("172.")) {
      return address;
    }
  }

  // 如果没有找到内网地址，返回第一个可用地址
  if (!addresses.isEmpty()) {
    return addresses.first();
  }

  return "127.0.0.1"; // 备用方案
}

// quint16 NetworkUtil::getAvailablePort(quint16 startPort, quint16 endPort) {
//   // 方法1：顺序查找
//   for (quint16 port = startPort; port <= endPort; ++port) {
//     if (isPortAvailable(port)) {
//       return port;
//     }
//   }

//   // 方法2：如果顺序查找失败，让系统自动分配端口
//   QTcpServer server;
//   if (server.listen(QHostAddress::Any, 0)) { // 0表示让系统自动分配
//     quint16 systemPort = server.serverPort();
//     server.close();

//     // 检查系统分配的端口是否在我们的范围内
//     if (systemPort >= startPort && systemPort <= endPort) {
//       return systemPort;
//     }

//     // 如果不在范围内，但是可用，也返回它
//     return systemPort;
//   }

//   return 0; // 没有找到可用端口
// }

// bool NetworkUtil::isPortAvailable(quint16 port) {
//   // 方法1：使用QTcpServer（当前方法）
//   QTcpServer server;
//   bool result = server.listen(QHostAddress::Any, port);
//   if (result) {
//     server.close();
//     return true;
//   }

//   // 如果失败，检查错误原因
//   QAbstractSocket::SocketError error = server.serverError();

//   // 如果是权限问题，尝试绑定到本地地址
//   if (error == QAbstractSocket::SocketAccessError) {
//     QTcpServer localServer;
//     bool localResult = localServer.listen(QHostAddress::LocalHost, port);
//     if (localResult) {
//       localServer.close();
//       return true;
//     }
//   }

//   return false;
// }

// QString NetworkUtil::getNetworkInterfaceInfo() {
//   QString info;
//   const QList<QNetworkInterface> interfaces =
//       QNetworkInterface::allInterfaces();

//   for (const QNetworkInterface &interface : interfaces) {
//     if (!(interface.flags() & QNetworkInterface::IsUp) ||
//         !(interface.flags() & QNetworkInterface::IsRunning)) {
//       continue;
//     }

//     info += QString("接口: %1\n").arg(interface.humanReadableName());
//     info += QString("MAC地址: %1\n").arg(interface.hardwareAddress());

//     const QList<QNetworkAddressEntry> entries = interface.addressEntries();
//     for (const QNetworkAddressEntry &entry : entries) {
//       const QHostAddress address = entry.ip();
//       if (address.protocol() == QAbstractSocket::IPv4Protocol) {
//         info += QString("IP地址: %1\n").arg(address.toString());
//         info += QString("子网掩码: %1\n").arg(entry.netmask().toString());
//       }
//     }
//     info += "\n";
//   }

//   return info;
// }

quint16 NetworkUtil::getSystemAssignedPort() {
  QTcpServer server;
  if (server.listen(QHostAddress::Any, 0)) { // 0让系统自动分配
    quint16 port = server.serverPort();
    server.close();
    return port;
  }
  return 0;
}
