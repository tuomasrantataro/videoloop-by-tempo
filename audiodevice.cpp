#include "audiodevice.h"
#include <QList>

AudioDevice::AudioDevice(QObject *parent, QString defaultDevice) : parent(parent), m_deviceName(defaultDevice)
{
    m_pullTimer = new QTimer();
    m_pullTimer->setInterval(1000);
    connect(m_pullTimer, &QTimer::timeout, this, &AudioDevice::writeToBuffer);

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    //qWarning("%d audio input devices found.", devices.count());

    for (auto it = devices.begin(); it != devices.end(); it++) {
        if (it->deviceName().compare(defaultDevice) == 0) {
            m_monitors.push_front(*it);
        } else if (it->deviceName().endsWith("monitor")) {
            m_monitors.append(*it);
        }
    }

    if (m_monitors.count() == 0) {
        qWarning("Default device or audio monitors were not found. Using system default audio input.");
        m_monitors.append(QAudioDeviceInfo::defaultInputDevice());
    }

    qWarning("Monitors found:");
    for (auto it = m_monitors.begin(); it != m_monitors.end(); it++) {
        qWarning("%s", qPrintable(it->deviceName()));
    }

    m_device = m_monitors[0];

    qWarning("Using audio output: %s", qPrintable(m_device.deviceName()));

    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleSize(8);
    m_format.setSampleType(QAudioFormat::UnSignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    QAudioDeviceInfo devInfo(m_device);
    if (!devInfo.isFormatSupported(m_format)) {
        qWarning("Default audio format not supported - trying to use nearest.");
        m_format = devInfo.nearestFormat(m_format);
    }

    m_audioDataHandler = new AudioDataHandler(m_format);
    m_audioInput = new QAudioInput(m_device, m_format, this);
    connect(m_audioDataHandler, &AudioDataHandler::dataReady, this, &AudioDevice::dataReady);

    m_audioDataHandler->start();
    m_audioInput->start(m_audioDataHandler);
    m_input = m_audioInput->start();
    m_pullTimer->start();

}

AudioDevice::~AudioDevice()
{
    delete m_audioDataHandler;
}

void AudioDevice::changeAudioInput(QString deviceName)
{
    qWarning("Not implemented: Change audio device to %s", qPrintable(deviceName));
    return;
}

void AudioDevice::writeToBuffer()
{
    int length = m_audioInput->bytesReady();
    if (length > 0) {
        m_audioDataHandler->writeData(m_input->readAll(), length);
    }
    return;
}

