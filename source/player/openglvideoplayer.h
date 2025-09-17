#ifndef OPENGLVIDEOPLAYER_H
#define OPENGLVIDEOPLAYER_H

#include <QImage>
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QTimer>

class OpenGLVideoPlayer : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit OpenGLVideoPlayer(QWidget *parent = nullptr);
  ~OpenGLVideoPlayer();

  // 更新视频帧
  void updateFrame(const QImage &frame);
  void updateFrame(const uchar *data, int width, int height,
                   QImage::Format format = QImage::Format_RGB888);

  // 设置视频尺寸
  void setVideoSize(int width, int height);

  // 开始/停止模拟视频流
  void startDemo();
  void stopDemo();

  // 获取视频信息
  int videoWidth() const { return m_videoWidth; }
  int videoHeight() const { return m_videoHeight; }
  bool hasFrame() const { return m_hasFrame; }

signals:
  void frameUpdated();
  void videoSizeChanged(int width, int height);

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;

private slots:
  void generateDemoFrame();
  void updateTexture();

private:
  void setupShaders();
  void setupVertexBuffer();
  QImage generateTestPattern();

private:
  // OpenGL 相关
  QOpenGLShaderProgram *m_program;
  QOpenGLTexture *m_texture;
  QOpenGLBuffer m_vertexBuffer;
  QOpenGLVertexArrayObject m_vao;

  // 视频数据
  QImage m_currentFrame;
  QMutex m_frameMutex;

  // 视频信息
  int m_videoWidth;
  int m_videoHeight;
  bool m_hasFrame;

  // 演示模式
  QTimer *m_demoTimer;
  int m_demoFrameCount;

  // 着色器代码
  static const char *vertexShaderSource;
  static const char *fragmentShaderSource;
};

#endif // OPENGLVIDEOPLAYER_H