#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QAudioDeviceInfo>
#include <QAudio>
#include <QAudioFormat>
#include <QAudioInput>
#include <QList>
#include <vector>
#include "audiodatahandler.h"

class AudioDevice : public QObject
{
    Q_OBJECT
public:
    AudioDevice(QObject *parent, QString defaultDevice);
    ~AudioDevice();

public slots:
    void changeAudioInput(QString inputName);
    void writeToBuffer();

signals:
    void dataReady(std::vector<uint8_t> data);
    void audioInputs(QObject*);

private:
    QObject *parent;
    QString m_deviceName;
    QTimer *m_pullTimer;

    QList<QAudioDeviceInfo> m_monitors;
    QAudioDeviceInfo m_device;

    QAudioFormat m_format;

    QAudioInput *m_audioInput;
    QIODevice *m_input;
    AudioDataHandler *m_audioDataHandler;
    
};

#endif