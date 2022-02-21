#include "graphicswidget.h"

GraphicsWidget::GraphicsWidget(QString videoLoop)
{
    m_modelLoader = new ModelLoader();


    m_frameLoader = new FrameLoader("./assets/frames", 1920, 1080);

    // 1. get max frame number
    // 2. load the startup loop frames
    // 3. load the startup model (square)
    // 4. create opengl shit with these
    // 5. load rest of the frames
    // 6. load rest of the models
    // 7. update

    getAllModels();
    getAllLoops();
    

    // Create widget and layout showing the 3D graphics
    int maxFrames = m_frameLoader->getMaxFrames();
    m_openGLWidget = new OpenGLWidget2(maxFrames);
    m_openGLWidget->setParent(this);
    ModelData first = m_models["square.obj"];
    m_openGLWidget->setBufferData(first.vertices, first.normals, first.textureUV, first.indices);
    m_openGLWidget->setNodeData(first.rootNode);
    m_openGLWidget->setDrawSquare(true);

    if (m_frameDict.contains(videoLoop)) {
        setVideoLoop(videoLoop);
    }
    else {
        setVideoLoop(m_frameDict.firstKey());
    }

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_openGLWidget);
    setLayout(m_layout);

    m_timer = new QTimer();
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->start(10);

    connect(m_timer, &QTimer::timeout, this, &GraphicsWidget::nextFrame);

    connect(m_openGLWidget, &OpenGLWidget2::initDone, this, &GraphicsWidget::initDone);
}

GraphicsWidget::~GraphicsWidget()
{
    delete m_modelLoader;
    delete m_frameLoader;
    delete m_openGLWidget;
}

void GraphicsWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit toggleFullScreen();
}

void GraphicsWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        emit spacePressed();
    }
    if (e->key() == Qt::Key_Escape) {
        emit toggleFullScreen();
    }
}

void GraphicsWidget::calculateFrameIndex()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - m_startTime;
    m_currentFrameIndex = int(elapsed.count()/m_timePerFrame_s) % m_frameCount;
}

void GraphicsWidget::nextFrame()
{
    calculateFrameIndex();
    int realIndex = m_frameIndexes.at(m_currentFrameIndex);
    m_openGLWidget->setFrameIndex(realIndex);
    m_openGLWidget->repaint();
}

void GraphicsWidget::addReversedFrames(bool set)
{
    m_addReversedFrames = set;
    if (set) {
        m_frameCount = m_frameIndexes.size();
    }
    else {
        m_frameCount = m_currentFrames.size();
    }
}

void GraphicsWidget::setTempo(double tempo)
{
    if (m_frameCount > 0) {
        m_timePerFrame_s = (60.0/tempo)/double(m_frameCount);
        m_startTime = std::chrono::high_resolution_clock::now() - std::chrono::microseconds(int(m_currentFrameIndex*1000000*m_timePerFrame_s));

    }
}

void GraphicsWidget::setVideoLoop(QString loopName)
{
    if (!m_frameDict.contains(loopName)) {
        qDebug() << "loop not found:" << loopName;
        return;
    }

    m_currentFrames = m_frameDict.value(loopName);
    m_frameCount = m_currentFrames.size();

    std::vector<int> indexes;

    for (int i = 0; i < m_frameCount; i++) {
        indexes.push_back(i);
    }

    for (int i = m_frameCount-1; i > 0; i--) {
        indexes.push_back(i);
    }

    m_frameIndexes = indexes;

    addReversedFrames(m_addReversedFrames);
    m_openGLWidget->setFrames(m_currentFrames);
}

void GraphicsWidget::getAllLoops()
{
    QString frameFolderPath = "./assets/frames/";
    QDir framesDir(frameFolderPath);

    QStringList frameFolders = framesDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);

    for (QString folder : frameFolders) {
        QVector<QImage> frames;

        QString path = frameFolderPath + folder;

        m_frameLoader->loadFrames(folder, &frames);

        m_frameDict.insert(folder, frames);
    }

}

void GraphicsWidget::getAllModels()
{
    QString modelFolderPath = "./assets/models/";
    QDir modelDir(modelFolderPath);

    QStringList models = modelDir.entryList(QDir::Files);


    for (QString modelName : models) {
        ModelData model;

        QString path = modelFolderPath + modelName;

        model = m_modelLoader->getModelData(path);

        if (model.textureUV.size() == 0) {
            qDebug() << "no textureUV map for" << modelName << ", skipping";
            continue;
        }

        m_models.insert(modelName, model);
    }

}