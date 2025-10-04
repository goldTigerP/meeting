#include "discovery_test_window.h"
#include <QApplication>
#include <QHostAddress>
#include <QDateTime>
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>
#include <QTextCursor>

DiscoveryTestWindow::DiscoveryTestWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_discoveryService(nullptr)
    , m_isStarted(false)
{
    setWindowTitle("设备发现测试 - Meeting");
    setMinimumSize(800, 600);
    
    // 创建发现服务
    m_discoveryService = new DiscoveryService(this);
    
    // 连接信号
    connect(m_discoveryService, &DiscoveryService::deviceDiscovered,
            this, &DiscoveryTestWindow::onDeviceDiscovered);
    connect(m_discoveryService, &DiscoveryService::deviceLeft,
            this, &DiscoveryTestWindow::onDeviceLeft);
    connect(m_discoveryService, &DiscoveryService::communicationAddressReceived,
            this, &DiscoveryTestWindow::onCommunicationAddressReceived);
    connect(m_discoveryService, &DiscoveryService::errorOccurred,
            this, &DiscoveryTestWindow::onErrorOccurred);
    
    // 设置UI
    setupUI();
    
    // 初始状态
    updateStatus("未启动");
}

DiscoveryTestWindow::~DiscoveryTestWindow()
{
    if (m_isStarted) {
        m_discoveryService->stopDiscovery();
    }
}

void DiscoveryTestWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    
    // 控制区域
    QGroupBox* controlGroup = new QGroupBox("控制面板", this);
    m_controlLayout = new QHBoxLayout(controlGroup);
    
    QLabel* nameLabel = new QLabel("设备名称:", this);
    m_deviceNameEdit = new QLineEdit(this);
    m_deviceNameEdit->setText("测试设备-" + QString::number(QDateTime::currentMSecsSinceEpoch() % 1000));
    m_deviceNameEdit->setMaximumWidth(200);
    
    m_startButton = new QPushButton("启动发现", this);
    m_stopButton = new QPushButton("停止发现", this);
    m_refreshButton = new QPushButton("刷新列表", this);
    
    m_stopButton->setEnabled(false);
    
    m_controlLayout->addWidget(nameLabel);
    m_controlLayout->addWidget(m_deviceNameEdit);
    m_controlLayout->addWidget(m_startButton);
    m_controlLayout->addWidget(m_stopButton);
    m_controlLayout->addWidget(m_refreshButton);
    m_controlLayout->addStretch();
    
    // 状态显示
    m_statusLabel = new QLabel("状态: 未启动", this);
    m_statusLabel->setStyleSheet("QLabel { font-weight: bold; color: #666; }");
    
    // 主要内容区域 - 使用分割器
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // 设备列表
    QGroupBox* deviceGroup = new QGroupBox("发现的设备", this);
    QVBoxLayout* deviceLayout = new QVBoxLayout(deviceGroup);
    m_deviceListWidget = new QListWidget(this);
    deviceLayout->addWidget(m_deviceListWidget);
    
    // 日志区域
    QGroupBox* logGroup = new QGroupBox("发现日志", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    logLayout->addWidget(m_logTextEdit);
    
    splitter->addWidget(deviceGroup);
    splitter->addWidget(logGroup);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    
    // 布局组装
    m_mainLayout->addWidget(controlGroup);
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addWidget(splitter);
    
    // 连接信号
    connect(m_startButton, &QPushButton::clicked, this, &DiscoveryTestWindow::startDiscovery);
    connect(m_stopButton, &QPushButton::clicked, this, &DiscoveryTestWindow::stopDiscovery);
    connect(m_refreshButton, &QPushButton::clicked, this, &DiscoveryTestWindow::refreshDeviceList);
    
    // 添加初始日志
    m_logTextEdit->append("=== 设备发现测试窗口已启动 ===");
    m_logTextEdit->append("请输入设备名称并点击'启动发现'开始测试");
}

void DiscoveryTestWindow::startDiscovery()
{
    QString deviceName = m_deviceNameEdit->text().trimmed();
    if (deviceName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入设备名称！");
        return;
    }
    
    // 启动发现服务
    if (m_discoveryService->startDiscovery(deviceName)) {
        m_isStarted = true;
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_deviceNameEdit->setEnabled(false);
        
        updateStatus("正在运行 - 每秒发送心跳");
        
        QString logMsg = QString("[%1] 启动设备发现服务 - 设备名称: %2")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                        .arg(deviceName);
        m_logTextEdit->append(logMsg);
        
        // 显示配置信息
        m_logTextEdit->append("配置信息:");
        m_logTextEdit->append("  预设组播地址: 239.255.43.21:45454");
        m_logTextEdit->append("  通信组播地址: 239.255.43.22:45455");
        m_logTextEdit->append("  心跳间隔: 1秒");
        
    } else {
        QMessageBox::critical(this, "错误", "启动设备发现服务失败！");
    }
}

void DiscoveryTestWindow::stopDiscovery()
{
    m_discoveryService->stopDiscovery();
    
    m_isStarted = false;
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_deviceNameEdit->setEnabled(true);
    
    updateStatus("已停止");
    
    QString logMsg = QString("[%1] 停止设备发现服务")
                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    m_logTextEdit->append(logMsg);
    
    // 清空设备列表
    m_deviceListWidget->clear();
}

void DiscoveryTestWindow::onDeviceDiscovered(const QString& deviceId, const QString& deviceName, const QHostAddress& address)
{
    QString logMsg = QString("[%1] 发现新设备: %2 (%3) - IP: %4")
                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                    .arg(deviceName)
                    .arg(deviceId.mid(0, 8) + "...")  // 只显示ID的前8位
                    .arg(address.toString());
    m_logTextEdit->append(logMsg);
    
    // 刷新设备列表
    refreshDeviceList();
    
    // 滚动到底部
    m_logTextEdit->moveCursor(QTextCursor::End);
}

void DiscoveryTestWindow::onDeviceLeft(const QString& deviceId)
{
    QString logMsg = QString("[%1] 设备离线: %2")
                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                    .arg(deviceId.mid(0, 8) + "...");
    m_logTextEdit->append(logMsg);
    
    // 刷新设备列表
    refreshDeviceList();
    
    // 滚动到底部
    m_logTextEdit->moveCursor(QTextCursor::End);
}

void DiscoveryTestWindow::onCommunicationAddressReceived(const QString& deviceId, const QHostAddress& multicastAddress, quint16 port)
{
    QString logMsg = QString("[%1] 接收到设备 %2 的通信地址: %3:%4")
                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                    .arg(deviceId.mid(0, 8) + "...")
                    .arg(multicastAddress.toString())
                    .arg(port);
    m_logTextEdit->append(logMsg);
    
    // 滚动到底部
    m_logTextEdit->moveCursor(QTextCursor::End);
}

void DiscoveryTestWindow::onErrorOccurred(const QString& error)
{
    QString logMsg = QString("[%1] 错误: %2")
                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                    .arg(error);
    m_logTextEdit->append(logMsg);
    m_logTextEdit->moveCursor(QTextCursor::End);
    
    QMessageBox::critical(this, "发现服务错误", error);
}

void DiscoveryTestWindow::refreshDeviceList()
{
    m_deviceListWidget->clear();
    
    QList<DiscoveryService::DeviceInfo> devices = m_discoveryService->getOnlineDevices();
    
    for (const auto& device : devices) {
        QString itemText = QString("%1 (%2)\nIP: %3\n通信地址: %4:%5")
                          .arg(device.deviceName)
                          .arg(device.deviceId.mid(0, 8) + "...")
                          .arg(device.address.toString())
                          .arg(device.communicationMulticastAddress.toString())
                          .arg(device.communicationPort);
        
        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, device.deviceId);
        m_deviceListWidget->addItem(item);
    }
    
    // 更新状态
    if (m_isStarted) {
        updateStatus(QString("正在运行 - 发现 %1 个设备").arg(devices.size()));
    }
}

void DiscoveryTestWindow::updateStatus(const QString& status)
{
    m_statusLabel->setText("状态: " + status);
}