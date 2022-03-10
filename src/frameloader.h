#ifndef FRAMELOADER_H
#define FRAMELOADER_H

#include <string>
#include <vector>
#include <QtCore>
#include <QImage>

#include <QThread>

class ReadWorker;

class FrameLoader : public QObject
{
    Q_OBJECT
public:

    FrameLoader(QString frameFolderPath, int width = 1920, int height = 1080);
    ~FrameLoader();

    int getMaxFrames();
    int getFrameCount(QString folderName);
    QStringList getFrameFolderNames() { return m_frameFolderNames; };

    void loadFrames(QString folderName, QVector<QImage> *frames, int amount = 0);

signals:
    void loadFramesAsync(QString folderName);

    void firstFramesReady(QString folderName, QVector<QImage> frames);
    void framesReady(QString folderName, QVector<QImage> frames);

private:
    QString m_frameFolderPath;
    int m_width;
    int m_height;

    QStringList m_frameFolderNames;

    QImage scaleFrame(QImage frame);

    ReadWorker* m_worker;
    QThread m_workerThread;
};


class ReadWorker : public QObject
{
    Q_OBJECT
public:
    ReadWorker(QString frameFolderPath);

public slots:
    void readFrames(QString fileName);

signals:
    void readReady(QString folderName, QVector<QImage> frames);
    void firstFramesReady(QString folderName, QVector<QImage> frames);

private:
    QString m_frameFolderPath;
};

#endif