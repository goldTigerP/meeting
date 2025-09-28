#ifndef NETWORK_UTIL_H
#define NETWORK_UTIL_H

#include <QHostAddress>
#include <QObject>
#include <QString>
#include <QStringList>

class NetworkUtil {

public:
  // 获取本机所有IP地址
  static QStringList getLocalIPAddresses();

  // 获取本机主要IP地址（排除本地回环地址）
  static QString getPrimaryIPAddress();

  //   // 分配一个可用的端口号
  //   static quint16 getAvailablePort(quint16 startPort = 8000,
  //                                   quint16 endPort = 9000);

  //   // 检查指定端口是否可用
  //   static bool isPortAvailable(quint16 port);

  // 让系统自动分配可用端口
  static quint16 getSystemAssignedPort();

  //   // 获取网络接口信息
  //   static QString getNetworkInterfaceInfo();
};

#endif // NETWORKUTIL_H