#include "openglvideoplayer.h"
#include <QDateTime>
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QRandomGenerator>
#include <cmath>
#include <qmath.h>

// 顶点着色器
const char *OpenGLVideoPlayer::vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}
)";

// 片段着色器
const char *OpenGLVideoPlayer::fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}
)";

OpenGLVideoPlayer::OpenGLVideoPlayer(QWidget *parent)
    : QOpenGLWidget(parent), m_program(nullptr), m_texture(nullptr),
      m_videoWidth(640), m_videoHeight(480), m_hasFrame(false),
      m_demoTimer(new QTimer(this)), m_demoFrameCount(0) {
  // 设置 OpenGL 格式
  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(format);

  // 设置最小尺寸
  setMinimumSize(320, 240);

  // 连接演示定时器
  connect(m_demoTimer, &QTimer::timeout, this,
          &OpenGLVideoPlayer::generateDemoFrame);

  qDebug() << "OpenGLVideoPlayer created";
}

OpenGLVideoPlayer::~OpenGLVideoPlayer() {
  makeCurrent();

  if (m_program) {
    delete m_program;
  }

  if (m_texture) {
    delete m_texture;
  }

  doneCurrent();

  qDebug() << "OpenGLVideoPlayer destroyed";
}

void OpenGLVideoPlayer::initializeGL() {
  initializeOpenGLFunctions();

  // 设置背景色
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  // 创建着色器程序
  setupShaders();

  // 设置顶点缓冲
  setupVertexBuffer();

  qDebug() << "OpenGL initialized successfully";
  qDebug() << "OpenGL Version:" << (char *)glGetString(GL_VERSION);
  qDebug() << "GLSL Version:"
           << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
}

void OpenGLVideoPlayer::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);

  if (!m_hasFrame || !m_texture || !m_program) {
    return;
  }

  m_program->bind();

  // 绑定纹理
  glActiveTexture(GL_TEXTURE0);
  m_texture->bind();
  m_program->setUniformValue("ourTexture", 0);

  // 绘制四边形
  m_vao.bind();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  m_vao.release();

  m_program->release();
}

void OpenGLVideoPlayer::resizeGL(int width, int height) {
  glViewport(0, 0, width, height);
}

void OpenGLVideoPlayer::setupShaders() {
  m_program = new QOpenGLShaderProgram(this);

  // 编译顶点着色器
  if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                          vertexShaderSource)) {
    qWarning() << "Failed to compile vertex shader:" << m_program->log();
    return;
  }

  // 编译片段着色器
  if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                          fragmentShaderSource)) {
    qWarning() << "Failed to compile fragment shader:" << m_program->log();
    return;
  }

  // 链接着色器程序
  if (!m_program->link()) {
    qWarning() << "Failed to link shader program:" << m_program->log();
    return;
  }

  qDebug() << "Shaders compiled and linked successfully";
}

void OpenGLVideoPlayer::setupVertexBuffer() {
  // 定义四边形顶点（位置 + 纹理坐标）
  float vertices[] = {
      // 位置          // 纹理坐标
      1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // 右上
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // 右下
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 左下
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f  // 左上
  };

  unsigned int indices[] = {
      0, 1, 3, // 第一个三角形
      1, 2, 3  // 第二个三角形
  };

  // 创建 VAO
  m_vao.create();
  m_vao.bind();

  // 创建 VBO
  m_vertexBuffer.create();
  m_vertexBuffer.bind();
  m_vertexBuffer.allocate(vertices, sizeof(vertices));

  // 创建 EBO
  GLuint EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // 设置顶点属性
  // 位置属性
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // 纹理坐标属性
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  m_vao.release();

  qDebug() << "Vertex buffer setup complete";
}

void OpenGLVideoPlayer::updateFrame(const QImage &frame) {
  QMutexLocker locker(&m_frameMutex);

  if (frame.isNull()) {
    return;
  }

  m_currentFrame = frame.rgbSwapped(); // OpenGL 使用 RGB 而不是 BGR

  // 更新视频尺寸
  if (m_videoWidth != frame.width() || m_videoHeight != frame.height()) {
    m_videoWidth = frame.width();
    m_videoHeight = frame.height();
    emit videoSizeChanged(m_videoWidth, m_videoHeight);
  }

  m_hasFrame = true;

  // 在主线程中更新纹理
  QMetaObject::invokeMethod(this, "updateTexture", Qt::QueuedConnection);
}

void OpenGLVideoPlayer::updateFrame(const uchar *data, int width, int height,
                                    QImage::Format format) {
  if (!data || width <= 0 || height <= 0) {
    return;
  }

  QImage frame(data, width, height, format);
  updateFrame(frame);
}

void OpenGLVideoPlayer::setVideoSize(int width, int height) {
  if (width > 0 && height > 0) {
    m_videoWidth = width;
    m_videoHeight = height;
    emit videoSizeChanged(width, height);
  }
}

void OpenGLVideoPlayer::updateTexture() {
  if (!m_hasFrame || m_currentFrame.isNull()) {
    return;
  }

  makeCurrent();

  QMutexLocker locker(&m_frameMutex);

  // 删除旧纹理
  if (m_texture) {
    delete m_texture;
  }

  // 创建新纹理
  m_texture = new QOpenGLTexture(m_currentFrame.mirrored());
  m_texture->setMinificationFilter(QOpenGLTexture::Linear);
  m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
  m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);

  doneCurrent();

  // 触发重绘
  update();

  emit frameUpdated();
}

void OpenGLVideoPlayer::startDemo() {
  qDebug() << "Starting demo mode";
  m_demoFrameCount = 0;
  m_demoTimer->start(33); // ~30 FPS
}

void OpenGLVideoPlayer::stopDemo() {
  qDebug() << "Stopping demo mode";
  m_demoTimer->stop();
}

void OpenGLVideoPlayer::generateDemoFrame() {
  QImage frame = generateTestPattern();
  updateFrame(frame);
  m_demoFrameCount++;
}

QImage OpenGLVideoPlayer::generateTestPattern() {
  QImage image(m_videoWidth, m_videoHeight, QImage::Format_RGB888);
  QPainter painter(&image);

  // 设置背景渐变
  QLinearGradient gradient(0, 0, m_videoWidth, m_videoHeight);
  gradient.setColorAt(0,
                      QColor::fromHsv((m_demoFrameCount * 2) % 360, 100, 150));
  gradient.setColorAt(
      1, QColor::fromHsv((m_demoFrameCount * 2 + 180) % 360, 100, 200));
  painter.fillRect(image.rect(), gradient);

  // 绘制时间信息
  painter.setPen(Qt::white);
  painter.setFont(QFont("Arial", 16, QFont::Bold));
  QString timeText = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
  painter.drawText(10, 30, timeText);

  // 绘制帧计数
  painter.drawText(10, 60, QString("Frame: %1").arg(m_demoFrameCount));

  // 绘制分辨率信息
  painter.drawText(
      10, 90,
      QString("Resolution: %1x%2").arg(m_videoWidth).arg(m_videoHeight));

  // 绘制移动的圆形
  int circleX = (m_demoFrameCount * 3) % (m_videoWidth + 100) - 50;
  int circleY = m_videoHeight / 2 + sin(m_demoFrameCount * 0.1) * 50;
  painter.setBrush(QColor(255, 255, 255, 150));
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(circleX - 25, circleY - 25, 50, 50);

  // 绘制网格
  painter.setPen(QPen(QColor(255, 255, 255, 50), 1));
  for (int x = 0; x < m_videoWidth; x += 50) {
    painter.drawLine(x, 0, x, m_videoHeight);
  }
  for (int y = 0; y < m_videoHeight; y += 50) {
    painter.drawLine(0, y, m_videoWidth, y);
  }

  return image;
}