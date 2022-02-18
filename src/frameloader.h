#ifndef FRAMELOADER_H
#define FRAMELOADER_H

#include <string>
#include <vector>
#include <QtCore>
#include <QImage>


class FrameLoader : public QObject
{
    Q_OBJECT
public:

    FrameLoader(QString frameFolderPath, int width = 1920, int height = 1080);

    int getMaxFrames();
    QStringList getFrameFolderNames() { return m_frameFolderNames; };

    void loadFrames(QString folderName, QVector<QImage> *frames);

private:
    QString m_frameFolderPath;
    int m_width;
    int m_height;

    QStringList m_frameFolderNames;

    QImage scaleFrame(QImage frame);
};

#endif