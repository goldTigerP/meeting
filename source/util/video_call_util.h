#ifndef VIDEO_CALL_UTIL_H
#define VIDEO_CALL_UTIL_H

#include <QObject>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QVideoWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QBuffer>
#include <QPixmap>

/**
 * 视频通话工具类
 * 功能：
 * 1. 本地摄像头预览
 * 2. 简单的P2P视频传输（基于TCP/UDP）
 * 3. 音视频设备管理
 * 4. 连接管理和信令
 */
class VideoCallUtil : public QObject
{
    Q_OBJECT

public:
    enum CallState {
        Idle,           // 空闲
        Calling,        // 呼叫中
        Ringing,        // 响铃中
        Connected,      // 已连接
        Disconnected    // 已断开
    };

    enum VideoQuality {
        Low,    // 320x240
        Medium, // 640x480
        High    // 1280x720
    };

    explicit VideoCallUtil(QObject *parent = nullptr);
    ~VideoCallUtil();

    // 摄像头管理
    QStringList getAvailableCameras();
    bool startCamera(const QString &cameraName = QString());
    void stopCamera();
    bool isCameraActive() const;
    
    // 视频预览
    QVideoWidget* getLocalVideoWidget();
    QVideoWidget* getRemoteVideoWidget();
    
    // 通话控制
    bool startCall(const QString &remoteIP, quint16 port);
    bool answerCall();
    void endCall();
    bool isInCall() const;
    
    // 服务器模式（等待连接）
    bool startListening(quint16 port);
    void stopListening();
    
    // 视频质量设置
    void setVideoQuality(VideoQuality quality);
    VideoQuality getVideoQuality() const;
    
    // 音视频控制
    void setMicrophoneEnabled(bool enabled);
    void setCameraEnabled(bool enabled);
    bool isMicrophoneEnabled() const;
    bool isCameraEnabled() const;

signals:
    void callStateChanged(CallState state);
    void incomingCall(const QString &remoteIP);
    void callConnected();
    void callDisconnected();
    void errorOccurred(const QString &error);
    void videoFrameReceived();

private slots:
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onCaptureTimer();
    void onImageCaptured(int id, const QImage &image);

private:
    void setupCamera();
    void setupNetworking();
    void sendVideoFrame(const QImage &frame);
    void processReceivedData(const QByteArray &data);
    void setState(CallState newState);
    QSize getResolutionForQuality(VideoQuality quality);
    
    // 摄像头相关
    QCamera *m_camera;
    QCameraViewfinder *m_localViewfinder;
    QCameraImageCapture *m_imageCapture;
    QVideoWidget *m_localVideoWidget;
    QVideoWidget *m_remoteVideoWidget;
    
    // 网络相关
    QTcpServer *m_tcpServer;
    QTcpSocket *m_tcpSocket;
    QUdpSocket *m_udpSocket;
    
    // 状态管理
    CallState m_callState;
    VideoQuality m_videoQuality;
    bool m_microphoneEnabled;
    bool m_cameraEnabled;
    bool m_isListening;
    
    // 视频传输
    QTimer *m_captureTimer;
    QMutex m_frameMutex;
    int m_captureId;
    
    // 连接信息
    QString m_remoteIP;
    quint16 m_remotePort;
    quint16 m_localPort;
};

#endif // VIDEO_CALL_UTIL_H