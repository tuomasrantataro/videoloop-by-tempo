#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QtCore>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QList>
#include <QBuffer>

#include <QAudioDeviceInfo>
#include <QAudio>
#include <QAudioFormat>
#include <QAudioInput>

#include <vector>
#include <list>

#include "types.h"

using namespace MyTypes;

class AudioDevice : public QObject
{
    Q_OBJECT
public:
    int RING_BUFFER_SIZE = 7;   // how many seconds of data is stored in ring buffer

    AudioDevice(QObject *parent, QString defaultDevice, bool showAllInputs);
    ~AudioDevice();

    QStringList getAudioDevices();
    QString getCurrentDevice() { return m_device.deviceName(); }


public slots:
    void changeAudioInput(QString inputName);

    void emitAndClearSongBuffer();

signals:
    void dataReady(const AudioData &data, const AudioBufferType type = MyTypes::rolling);
    void songDataReady(const AudioData &data, const AudioBufferType type = MyTypes::track);
    void invalidateData(QString reason);


private:
    void setupDevice(QString deviceName);

    void processAudioIn();

    void updateShortBuffer(const QByteArray &data);
    void updateSongBuffer(const QByteArray &data);

    QObject *parent;

    QList<QAudioDeviceInfo> m_monitors;
    QAudioDeviceInfo m_device;
    
    QAudioFormat m_format;

    QAudioInput *m_audioIn = nullptr;
    QBuffer m_inputBuffer;

    bool m_showAllInputs;

    AudioData *m_wholeTrackData;
    std::list<AudioData> *m_shortDataBuffer;    // used as constant size ring buffer
    AudioData *m_shortData;

};

#endif