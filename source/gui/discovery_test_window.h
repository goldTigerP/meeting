#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include "../service/discovery_service.h"

/**
 * 设备发现测试窗口
 * 用于测试设备发现功能
 */
class DiscoveryTestWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DiscoveryTestWindow(QWidget *parent = nullptr);
    ~DiscoveryTestWindow();

private slots:
    void startDiscovery();
    void stopDiscovery();
    void onDeviceDiscovered(const QString& deviceId, const QString& deviceName, const QHostAddress& address);
    void onDeviceLeft(const QString& deviceId);
    void onCommunicationAddressReceived(const QString& deviceId, const QHostAddress& multicastAddress, quint16 port);
    void onErrorOccurred(const QString& error);
    void refreshDeviceList();

private:
    void setupUI();
    void updateStatus(const QString& status);
    
    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlLayout;
    
    QLineEdit* m_deviceNameEdit;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_refreshButton;
    
    QLabel* m_statusLabel;
    QTextEdit* m_logTextEdit;
    QListWidget* m_deviceListWidget;
    
    // 服务组件
    DiscoveryService* m_discoveryService;
    bool m_isStarted;
};