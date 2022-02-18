#include "frameloader.h"

FrameLoader::FrameLoader(QString frameFolderPath, int width, int height)
    : m_frameFolderPath(frameFolderPath), m_width(width), m_height(height)
{
    m_frameFolderNames = QDir(m_frameFolderPath).entryList(QDir::NoDotAndDotDot | QDir::Dirs);
}

int FrameLoader::getMaxFrames()
{
    int maxFrames = 0;
    for (QString folderName : m_frameFolderNames) {
        QString folderPath = m_frameFolderPath + '/' + folderName;

        int frameCount = QDir(folderPath).entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG", QDir::Files).count();
        if (frameCount > maxFrames) {
            maxFrames = frameCount;
        }
    }
    return maxFrames;
}

void FrameLoader::loadFrames(QString folderName, QVector<QImage> *frames)
{
    QString path = m_frameFolderPath + '/' + folderName;

    QStringList frameNames = QDir(path).entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG", QDir::Files);

    frames->reserve(frameNames.size());

    for (QString filename : frameNames) {
        QString filePath = path + '/' + filename;
        QImage frame = QImage(filePath) .convertToFormat(QImage::Format_RGBA8888)
                                        .mirrored();

        QImage scaledFrame = scaleFrame(frame);

        frames->push_back(scaledFrame);
    }
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