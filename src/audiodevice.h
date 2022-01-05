#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QtCore>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QAudioDeviceInfo>
#include <QAudio>
#include <QAudioFormat>
#include <QAudioInput>
#include <QList>
#include <QBuffer>

#include <vector>
#include <list>

class AudioDevice : public QObject
{
    Q_OBJECT
public:
    int RING_BUFFER_SIZE = 7;   // how many seconds of data is stored in ring buffer

    AudioDevice(QObject *parent, QString defaultDevice, bool showAllInputs);
    ~AudioDevice();

    QStringList getAudioDevices();
    QString getCurrentDevice() { return m_device.deviceName(); }

    void emitAndClearSongBuffer();

public slots:
    void changeAudioInput(QString inputName);

    void printStateChange(QAudio::State);

signals:
    void dataReady(std::vector<uint8_t> &data);
    void songDataReady(std::vector<uint8_t> &data);

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

    std::vector<uint8_t> *m_wholeTrackData;
    std::list<std::vector<uint8_t>> *m_shortDataBuffer;    // will be used as a constant size ring buffer
    std::vector<uint8_t> *m_shortData;

};

#endif