#include "openglwidget.h"
#include <QDirIterator>

OpenGLWidget::OpenGLWidget(QString frameFolder) : m_frameFolder(frameFolder)
{
    loadVideoFrameFiles();
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    delete m_timer;
    m_vbo->destroy();
    delete m_vbo;
    for (auto item : m_textures) {
        delete item;
    }
    delete m_program;
    doneCurrent();
}

void OpenGLWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit toggleFullScreen();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        emit spacePressed();
    }
    if (e->key() == Qt::Key_Escape) {
        emit toggleFullScreen();
    }
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    makeObject();

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec4 texCoord;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc.st);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    m_program = new QOpenGLShaderProgram;
    m_program->addShader(vshader);
    m_program->addShader(fshader);
    m_program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
    m_program->link();
    m_program->bind();
    m_program->setUniformValue("texture", 0);

    m_timer = new QTimer();
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->start(10);
    this->setBpm(60.0); // set some initial looping speed

    connect(m_timer, &QTimer::timeout, this, &OpenGLWidget::nextFrame);
    emit initReady();
}

void OpenGLWidget::paintGL()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float verticalScaling = m_frameAspectRatio/m_viewPortAspectRatio;
    float horizontalScaling = 1.0/verticalScaling;
    if (verticalScaling < 1.0) {
        verticalScaling = 1.0;
    } 
    else {
        horizontalScaling = 1.0;
    }
    QMatrix4x4 m;
    m.ortho(-horizontalScaling, horizontalScaling, -verticalScaling, verticalScaling, -1.0f, 1.0f);

    m_program->setUniformValue("matrix", m);
    m_program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    m_program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 2, 4 * sizeof(GLfloat));
    m_program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 2 * sizeof(GLfloat), 2, 4 * sizeof(GLfloat));

    int idx = m_frameIndexes[m_currentFrameIndex];
    m_textures.at(idx)->bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void OpenGLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
    m_viewPortAspectRatio = float(width)/float(height);
}

void OpenGLWidget::makeObject()
{
    m_frameIndexes.clear();

    VideoFrames loop = m_videoLoops.value(m_frameFolder);

    QList<QImage> frames = loop.frames;

    int imageCount = frames.size();

    m_frameIndexes = loop.frameIndexes;
    m_frameAspectRatio = loop.aspectRatio;

    for (int i = 0; i < imageCount; i++) {
        m_textures.append(new QOpenGLTexture(frames[i]));
    }

    m_frameCount = m_textures.size();

    static const int coords[4][2] = {
        { +1, -1 },
        { -1, -1 },
        { -1, +1 },
        { +1, +1 }
    };

    QVector<GLfloat> vertData;
    for (int j = 0; j < 4; ++j) {
        // vertex position
        vertData.append(coords[j][0]);
        vertData.append(coords[j][1]);
        // texture coordinate
        vertData.append(j == 0 || j == 3);
        vertData.append(j == 0 || j == 1);
        }

    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
}

void OpenGLWidget::setBpm(float tempo)
{
    m_timePerFrame_s = (60.0/tempo)/double(m_frameCount);
    m_startTime = std::chrono::high_resolution_clock::now() - std::chrono::microseconds(int(m_currentFrameIndex*1000000*m_timePerFrame_s));
}

void OpenGLWidget::nextFrame()
{
    calculateFrameIndex();
    update();
}

void OpenGLWidget::calculateFrameIndex()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - m_startTime;
    m_currentFrameIndex = int(elapsed.count()/m_timePerFrame_s) % m_frameCount;
}

void OpenGLWidget::setAddReversedFrames(bool add)
{
    m_addReversedFrames = add;
    if (add) {
        m_frameCount = m_frameIndexes.size();
    }
    else {
        m_frameCount = m_textures.size();
    }
}

void OpenGLWidget::setFrameFolder(QString folderName)
{
    makeCurrent();
    QList<QOpenGLTexture*> textures;

    VideoFrames loop = m_videoLoops.value(folderName);

    QList<QImage> frames = loop.frames;

    int imageCount = frames.size();

    
    for (int i = 0; i < imageCount; i++) {
        textures.append(new QOpenGLTexture(frames[i]));
    }

    QList<QOpenGLTexture*> old = m_textures;

    //m_timer->stop();
    m_frameFolder = folderName;
    m_textures = textures;
    m_frameAspectRatio = loop.aspectRatio;
    m_frameIndexes = loop.frameIndexes;
    setAddReversedFrames(m_addReversedFrames);
    //m_timer->start(10);

    for (auto item : old) {
        delete item;
    }
}

void OpenGLWidget::loadVideoFrameFiles()
{
    QDir directory("frames");

    QStringList frameFolders = directory.entryList(QDir::NoDotAndDotDot | QDir::Dirs);

    for (QString folder : frameFolders) {
        VideoFrames framesObj;
        framesObj.name = folder;

        QString path = "frames/" + folder + "/";
        QDir frameDir(path);

        QStringList frameNames = frameDir.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG", QDir::Files);

        framesObj.frames.reserve(frameNames.size());

        for (QString filename : frameNames) {
            QString filePath = path + filename;
            framesObj.frames.push_back(QImage(filePath).convertToFormat(QImage::Format_RGBA8888_Premultiplied));
        }

        int imageCount = framesObj.frames.size();

        framesObj.aspectRatio = double(framesObj.frames[0].width())/double(framesObj.frames[0].height());

        for (int i = 0; i < imageCount; i++) {
            framesObj.frameIndexes.push_back(i);
        }
        
        if (imageCount > 1) {
            for (int i = imageCount-2; i > 0; i--) {
                framesObj.frameIndexes.push_back(i);
            }
        }

        m_videoLoops.insert(folder, framesObj);

    }
}