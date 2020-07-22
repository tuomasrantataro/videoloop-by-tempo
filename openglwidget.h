#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QDir>
#include <QList>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QSize>
#include <chrono>
#include <vector>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    using QOpenGLWidget::QOpenGLWidget;
    ~OpenGLWidget();

    void setBpm(float tempo);

    QSize minimumSizeHint() const {
        return QSize(600, 400);
    }

signals:
    void toggleFullScreen();
    void initReady();
    void spacePressed();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    void makeObject();

    QList<QOpenGLTexture*> m_textures;
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer *m_vbo;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime = std::chrono::high_resolution_clock::now();

    QTimer *m_timer = nullptr;
    float m_timePerFrame_s = 0.01;
    // Helper to show the frames also backwards. Contains (0, 1, 2 ... frameCount-2, frameCount-1, frameCount-2 ... 2, 1)
    std::vector<int> m_frameIndexes;
    float m_frameAspectRatio = 1.0;   // width/height
    float m_viewPortAspectRatio = 1.0;
    int m_frameCount;
    int m_currentFrameIndex = 0;
    void nextFrame();
    void calculateFrameIndex();
};

#endif