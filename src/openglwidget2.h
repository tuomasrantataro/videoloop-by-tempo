#ifndef OPENGLWIDGET2_H
#define OPENGLWIDGET2_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QtGui>

#include "modelloader.h"   // for types

class OpenGLWidget2 : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT
public:
    OpenGLWidget2(int maxFrames);
    ~OpenGLWidget2();
    QSize minimumSizeHint() const { return QSize(400, 300); }

    
    void setBufferData( QVector<float>& vertices,
                        QVector<float>& normals,
                        QVector<QVector<float>>& textureUV,
                        QVector<unsigned int>& indices);
    void setFrames(const QVector<QImage>& frames);
    void setNodeData(QSharedPointer<Node> rootNode);
    void setDrawSquare(bool drawSquare) { m_drawSquare = drawSquare; };

    void cleanup();

    void setFrameIndex(int idx);

signals:
    void initDone();

protected:

    void resizeGL(int width, int height) override;
    void initializeGL() override;
    void paintGL() override;

private:
    int m_maxFrames;
    void createShaderProgram( QString vShader, QString fShader);
    void createBuffers();
    void createAttributes();
    void setupLightingAndMatrices();
    void initTextures();

    void draw();
    void drawNode(const Node *node, QMatrix4x4 objectMatrix);
    void drawSquare(const Node *node, QMatrix4x4 objectMatrix);
    void setMaterialUniforms(MaterialInfo &mater);

    QOpenGLShaderProgram m_shaderProgram;

    QOpenGLVertexArrayObject m_vao;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_textureUVBuffer;
    QOpenGLBuffer m_indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    QSharedPointer<Node> m_rootNode;

    QMatrix4x4 m_projection, m_view, m_model;

    LightInfo m_lightInfo;
    MaterialInfo m_materialInfo;

    bool m_error;

    QOpenGLTexture *m_texture = nullptr;

    QVector<QImage> m_frames;
    int m_frameCount = 0;
    float m_frameAspectRatio = 1.0;
    float m_viewPortAspectRatio = 1.0;
    QVector<float> m_vertices;
    QVector<float> m_normals;
    QVector<QVector<float>> m_textureUV;
    QVector<unsigned int> m_indices;

    bool m_initDone = false;

    LightInfo m_straightLight = {
        QVector4D(0.0, 0.0, 1.0, 0.0),
        QVector3D(1.0, 1.0, 1.0)
    };

    MaterialInfo m_noMaterial = {
        "NoMaterial",
        QVector3D( 0.1f, 0.1f, 0.1f ),
        QVector3D( .9f, .9f, .9f ),
        QVector3D( .2f, .2f, .2f ),
        50.0f
    };

    bool m_drawSquare = true;
};

#endif