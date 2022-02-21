#ifndef GRAPHICSWIDGET_H
#define GRAPFICSWIDGET_H

#include <QtCore>
#include <QWidget>
#include <QVBoxLayout>
#include <QOpenGLContext>

#include "modelloader.h"
#include "frameloader.h"
#include "openglwidget2.h"

/*
struct ModelData {
    QVector<float> vertices;
    QVector<float> normals;
    QVector<unsigned int> indices;

    QVector<QVector<float> > textureUV; // multiple channels
    QVector<unsigned int > textureUVComponents; // multiple channels

    QVector<QSharedPointer<MaterialInfo> > materials;
    QVector<QSharedPointer<Mesh> > meshes;
    QSharedPointer<Node> rootNode;
};*/

class GraphicsWidget : public QWidget
{
    Q_OBJECT
public:
    GraphicsWidget(QString videoLoop = "");
    ~GraphicsWidget();

    QSize minimumSizeHint() const { return QSize(400, 300); }

public slots:
    void setTempo(double tempo);
    void addReversedFrames(bool set);
    void setVideoLoop(QString loopName);

signals:
    void toggleFullScreen();
    void spacePressed();
    void initDone();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    ModelLoader* m_modelLoader;

    FrameLoader* m_frameLoader;

    // for 3d model
    QVector<float> m_vertices;
    QVector<float> m_normals;
    QVector<unsigned int> m_indices;

    QVector<QVector<float> > m_textureUV; // multiple channels
    //QVector<float> m_tangents;
    //QVector<float> m_bitangents;
    QVector<unsigned int > m_textureUVComponents; // multiple channels

    QVector<QSharedPointer<MaterialInfo> > m_materials;
    QVector<QSharedPointer<Mesh> > m_meshes;
    QSharedPointer<Node> m_rootNode;

    QMap<QString, ModelData> m_models;
    // end of 3d model

    QVector<QImage> m_currentFrames;
    QMap<QString, QVector<QImage>> m_frameDict;

    OpenGLWidget2* m_openGLWidget;

    QTimer *m_timer;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime = std::chrono::high_resolution_clock::now();
    float m_timePerFrame_s = 0.01;
    // Helper to show the frames also backwards. Contains (0, 1, 2 ... frameCount-2, frameCount-1, frameCount-2 ... 2, 1)
    std::vector<int> m_frameIndexes;

    int m_currentFrameIndex = 0;

    int m_frameCount = 0;
    bool m_addReversedFrames = false;

    void calculateFrameIndex();
    void nextFrame();

    void getAllLoops();
    void getAllModels();

    QVBoxLayout *m_layout;

    //int m_counter = 0;
};



#endif