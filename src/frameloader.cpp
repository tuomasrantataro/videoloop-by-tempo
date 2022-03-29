#include "frameloader.h"

ReadWorker::ReadWorker(QString frameFolderPath) : m_frameFolderPath(frameFolderPath)
{

}

FrameLoader::FrameLoader(QString frameFolderPath, int width, int height)
    : m_frameFolderPath(frameFolderPath), m_width(width), m_height(height)
{
    qRegisterMetaType<QVector<QImage>>("QVector<QImage>");

    m_frameFolderNames = QDir(m_frameFolderPath).entryList(QDir::NoDotAndDotDot | QDir::Dirs);

    m_worker = new ReadWorker(m_frameFolderPath);
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &FrameLoader::loadFramesAsync, m_worker, &ReadWorker::readFrames);
    connect(m_worker, &ReadWorker::readReady, this, &FrameLoader::framesReady);
    connect(m_worker, &ReadWorker::firstFramesReady, this, &FrameLoader::firstFramesReady);
    m_workerThread.start();
}

FrameLoader::~FrameLoader()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

int FrameLoader::getMaxFrames()
{
    int maxFrames = 0;
    for (QString folderName : m_frameFolderNames) {
        QString folderPath = m_frameFolderPath + '/' + folderName;

        int frameCount = getFrameCount(folderName);

        if (frameCount > maxFrames) {
            maxFrames = frameCount;
        }
    }
    return maxFrames;
}

int FrameLoader::getFrameCount(QString folderName)
{
    QString folderPath = m_frameFolderPath + '/' + folderName;

    QStringList fileNames = QDir(folderPath).entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG" << "*.gif" << ".*GIF", QDir::Files);
        
    int frameCount = fileNames.count();

    return frameCount;
        
}

void FrameLoader::loadFrames(QString folderName, QVector<QImage> *frames, int amount)
{
    frames->clear();
    QString path = m_frameFolderPath + '/' + folderName;

    QStringList frameNames = QDir(path).entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG", QDir::Files);

    frames->reserve(frameNames.size());

    int counter = amount;

    for (QString filename : frameNames) {
        QString filePath = path + '/' + filename;
        QImage frame = QImage(filePath) .convertToFormat(QImage::Format_RGBA8888)
                                        .mirrored();

        //QImage scaledFrame = scaleFrame(frame);

        frames->push_back(frame);

        // if amount <= 0 -> load all frames
        if (--counter == 0) {
            break;
        }
    }
}


void ReadWorker::readFrames(QString folderName)
{
    QString path = m_frameFolderPath + '/' + folderName;

    QStringList frameNames = QDir(path).entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG", QDir::Files);

    QVector<QImage> frames;
    frames.reserve(frameNames.size());

    for (QString filename : frameNames) {
        QString filePath = path + '/' + filename;
        QImage frame = QImage(filePath) .convertToFormat(QImage::Format_RGBA8888)
                                        .mirrored();

        //QImage scaledFrame = scaleFrame(frame);

        frames.push_back(frame);

    }
    emit readReady(folderName, frames);

}

QImage FrameLoader::scaleFrame(QImage frame)
{
    int width = frame.width();
    int height = frame.height();

    float aspectRatio = float(width)/float(height);
    float wantedAspectRatio = float(m_width)/float(m_height);

    if (aspectRatio > wantedAspectRatio) {
        return frame.scaledToWidth(m_width).copy(0, (height-m_height)/2, m_width, m_height);
    }
    else {
        return frame.scaledToHeight(m_height);
    }
}