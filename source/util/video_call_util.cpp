#include "video_call_util.h"
#include <QCameraInfo>
#include <QApplication>
#include <QDebug>
#include <QHostAddress>
#include <QDataStream>
#include <QImageWriter>
#include <QImageReader>

VideoCallUtil::VideoCallUtil(QObject *parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_localViewfinder(nullptr)
    , m_imageCapture(nullptr)
    , m_localVideoWidget(nullptr)
    , m_remoteVideoWidget(nullptr)
    , m_tcpServer(nullptr)
    , m_tcpSocket(nullptr)
    , m_udpSocket(nullptr)
    , m_callState(Idle)
    , m_videoQuality(Medium)
    , m_microphoneEnabled(true)
    , m_cameraEnabled(true)
    , m_isListening(false)
    , m_captureTimer(new QTimer(this))
    , m_captureId(0)
    , m_localPort(0)
{
    setupCamera();
    setupNetworking();
    
    // 设置视频捕获定时器（15 FPS）
    m_captureTimer->setInterval(66); // ~15 FPS
    connect(m_captureTimer, &QTimer::timeout, this, &VideoCallUtil::onCaptureTimer);
}

VideoCallUtil::~VideoCallUtil()
{
    stopCamera();
    endCall();
    stopListening();
}

void VideoCallUtil::setupCamera()
{
    // 创建本地和远程视频显示组件
    m_localVideoWidget = new QVideoWidget();
    m_localVideoWidget->setMinimumSize(320, 240);
    m_localVideoWidget->setStyleSheet("border: 2px solid #3498db; border-radius: 8px;");
    
    m_remoteVideoWidget = new QVideoWidget();
    m_remoteVideoWidget->setMinimumSize(640, 480);
    m_remoteVideoWidget->setStyleSheet("border: 2px solid #e74c3c; border-radius: 8px;");
    
    // 初始化摄像头（如果有可用的）
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (!cameras.isEmpty()) {
        startCamera(cameras.first().deviceName());
    }
}

void VideoCallUtil::setupNetworking()
{
    // 初始化TCP服务器
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &VideoCallUtil::onNewConnection);
    
    // 初始化UDP套接字（用于视频流）
    m_udpSocket = new QUdpSocket(this);
}

QStringList VideoCallUtil::getAvailableCameras()
{
    QStringList cameraNames;
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    
    for (const QCameraInfo &cameraInfo : cameras) {
        cameraNames << QString("%1 (%2)").arg(cameraInfo.description()).arg(cameraInfo.deviceName());
    }
    
    if (cameraNames.isEmpty()) {
        cameraNames << "没有检测到摄像头";
    }
    
    return cameraNames;
}

bool VideoCallUtil::startCamera(const QString &cameraName)
{
    stopCamera(); // 停止之前的摄像头
    
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QCameraInfo selectedCamera;
    
    // 查找指定的摄像头
    for (const QCameraInfo &cameraInfo : cameras) {
        if (cameraName.isEmpty() || cameraInfo.deviceName() == cameraName || 
            cameraInfo.description() == cameraName) {
            selectedCamera = cameraInfo;
            break;
        }
    }
    
    if (selectedCamera.isNull()) {
        emit errorOccurred("未找到指定的摄像头");
        return false;
    }
    
    try {
        m_camera = new QCamera(selectedCamera, this);
        
        // 设置视频分辨率
        QSize resolution = getResolutionForQuality(m_videoQuality);
        QCameraViewfinderSettings settings;
        settings.setResolution(resolution);
        settings.setMinimumFrameRate(15.0);
        settings.setMaximumFrameRate(30.0);
        m_camera->setViewfinderSettings(settings);
        
        // 创建图像捕获
        m_imageCapture = new QCameraImageCapture(m_camera, this);
        connect(m_imageCapture, QOverload<int, const QImage &>::of(&QCameraImageCapture::imageCaptured),
                this, &VideoCallUtil::onImageCaptured);
        
        // 设置viewfinder到本地视频组件
        m_camera->setViewfinder(m_localVideoWidget);
        
        m_camera->start();
        
        qDebug() << "摄像头启动成功:" << selectedCamera.description();
        return true;
        
    } catch (...) {
        emit errorOccurred("启动摄像头失败");
        return false;
    }
}

void VideoCallUtil::stopCamera()
{
    if (m_camera) {
        m_camera->stop();
        m_camera->deleteLater();
        m_camera = nullptr;
    }
    
    if (m_imageCapture) {
        m_imageCapture->deleteLater();
        m_imageCapture = nullptr;
    }
    
    m_captureTimer->stop();
}

bool VideoCallUtil::isCameraActive() const
{
    return m_camera && m_camera->state() == QCamera::ActiveState;
}

QVideoWidget* VideoCallUtil::getLocalVideoWidget()
{
    return m_localVideoWidget;
}

QVideoWidget* VideoCallUtil::getRemoteVideoWidget()
{
    return m_remoteVideoWidget;
}

bool VideoCallUtil::startCall(const QString &remoteIP, quint16 port)
{
    if (m_callState != Idle) {
        emit errorOccurred("当前正在通话中");
        return false;
    }
    
    setState(Calling);
    m_remoteIP = remoteIP;
    m_remotePort = port;
    
    // 创建TCP连接
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &VideoCallUtil::onSocketReadyRead);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &VideoCallUtil::onSocketDisconnected);
    
    m_tcpSocket->connectToHost(remoteIP, port);
    
    if (m_tcpSocket->waitForConnected(5000)) {
        setState(Connected);
        emit callConnected();
        
        // 开始视频捕获和传输
        if (isCameraActive()) {
            m_captureTimer->start();
        }
        
        qDebug() << "成功连接到:" << remoteIP << ":" << port;
        return true;
    } else {
        setState(Idle);
        emit errorOccurred("连接失败: " + m_tcpSocket->errorString());
        return false;
    }
}

bool VideoCallUtil::answerCall()
{
    if (m_callState != Ringing) {
        return false;
    }
    
    setState(Connected);
    emit callConnected();
    
    // 开始视频传输
    if (isCameraActive()) {
        m_captureTimer->start();
    }
    
    return true;
}

void VideoCallUtil::endCall()
{
    m_captureTimer->stop();
    
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->deleteLater();
        m_tcpSocket = nullptr;
    }
    
    setState(Idle);
    emit callDisconnected();
    
    qDebug() << "通话已结束";
}

bool VideoCallUtil::isInCall() const
{
    return m_callState == Connected;
}

bool VideoCallUtil::startListening(quint16 port)
{
    if (m_isListening) {
        stopListening();
    }
    
    if (m_tcpServer->listen(QHostAddress::Any, port)) {
        m_localPort = port;
        m_isListening = true;
        qDebug() << "开始监听端口:" << port;
        return true;
    } else {
        emit errorOccurred("无法监听端口: " + m_tcpServer->errorString());
        return false;
    }
}

void VideoCallUtil::stopListening()
{
    if (m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
    m_isListening = false;
}

void VideoCallUtil::setVideoQuality(VideoQuality quality)
{
    m_videoQuality = quality;
    
    // 如果摄像头正在运行，重启以应用新设置
    if (isCameraActive()) {
        QString currentCamera;
        if (m_camera) {
            // 保存当前摄像头信息
            // 重启摄像头以应用新分辨率
            startCamera(currentCamera);
        }
    }
}

VideoCallUtil::VideoQuality VideoCallUtil::getVideoQuality() const
{
    return m_videoQuality;
}

void VideoCallUtil::setMicrophoneEnabled(bool enabled)
{
    m_microphoneEnabled = enabled;
    // 这里可以添加麦克风控制逻辑
}

void VideoCallUtil::setCameraEnabled(bool enabled)
{
    m_cameraEnabled = enabled;
    
    if (!enabled) {
        m_captureTimer->stop();
    } else if (isInCall() && isCameraActive()) {
        m_captureTimer->start();
    }
}

bool VideoCallUtil::isMicrophoneEnabled() const
{
    return m_microphoneEnabled;
}

bool VideoCallUtil::isCameraEnabled() const
{
    return m_cameraEnabled;
}

void VideoCallUtil::onNewConnection()
{
    if (m_callState != Idle) {
        // 已经在通话中，拒绝新连接
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();
        socket->disconnectFromHost();
        socket->deleteLater();
        return;
    }
    
    m_tcpSocket = m_tcpServer->nextPendingConnection();
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &VideoCallUtil::onSocketReadyRead);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &VideoCallUtil::onSocketDisconnected);
    
    m_remoteIP = m_tcpSocket->peerAddress().toString();
    setState(Ringing);
    
    emit incomingCall(m_remoteIP);
    qDebug() << "收到来电:" << m_remoteIP;
}

void VideoCallUtil::onSocketReadyRead()
{
    if (!m_tcpSocket) return;
    
    QByteArray data = m_tcpSocket->readAll();
    processReceivedData(data);
}

void VideoCallUtil::onSocketDisconnected()
{
    setState(Idle);
    emit callDisconnected();
    
    if (m_tcpSocket) {
        m_tcpSocket->deleteLater();
        m_tcpSocket = nullptr;
    }
}

void VideoCallUtil::onCaptureTimer()
{
    if (m_imageCapture && isCameraActive() && m_cameraEnabled) {
        m_imageCapture->capture();
    }
}

void VideoCallUtil::onImageCaptured(int id, const QImage &image)
{
    Q_UNUSED(id)
    
    if (isInCall() && !image.isNull()) {
        sendVideoFrame(image);
    }
}

void VideoCallUtil::sendVideoFrame(const QImage &image)
{
    if (!m_tcpSocket || m_tcpSocket->state() != QTcpSocket::ConnectedState) {
        return;
    }
    
    // 压缩图像
    QSize targetSize = getResolutionForQuality(m_videoQuality);
    QImage scaledImage = image.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // 转换为JPEG格式以减少数据量
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    scaledImage.save(&buffer, "JPEG", 70); // 70% quality
    
    // 发送数据头（数据长度）+ 图像数据
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << quint32(imageData.size());
    packet.append(imageData);
    
    m_tcpSocket->write(packet);
}

void VideoCallUtil::processReceivedData(const QByteArray &data)
{
    static QByteArray buffer;
    static quint32 expectedSize = 0;
    
    buffer.append(data);
    
    while (buffer.size() >= sizeof(quint32)) {
        if (expectedSize == 0) {
            // 读取数据长度
            QDataStream stream(buffer);
            stream >> expectedSize;
            buffer.remove(0, sizeof(quint32));
        }
        
        if (buffer.size() >= expectedSize) {
            // 接收到完整的图像数据
            QByteArray imageData = buffer.left(expectedSize);
            buffer.remove(0, expectedSize);
            expectedSize = 0;
            
            // 解析并显示图像
            QImage image;
            if (image.loadFromData(imageData, "JPEG")) {
                // 在远程视频组件中显示
                QPixmap pixmap = QPixmap::fromImage(image);
                // 注意：QVideoWidget主要用于显示视频流，这里我们需要用其他方式显示图像
                // 可以考虑使用QLabel或自定义组件
                emit videoFrameReceived();
            }
        } else {
            break; // 等待更多数据
        }
    }
}

void VideoCallUtil::setState(CallState newState)
{
    if (m_callState != newState) {
        m_callState = newState;
        emit callStateChanged(newState);
    }
}

QSize VideoCallUtil::getResolutionForQuality(VideoQuality quality)
{
    switch (quality) {
    case Low:
        return QSize(320, 240);
    case Medium:
        return QSize(640, 480);
    case High:
        return QSize(1280, 720);
    default:
        return QSize(640, 480);
    }
}