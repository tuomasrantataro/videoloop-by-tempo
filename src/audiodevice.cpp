#include <QList>

#include "audiodevice.h"

AudioDevice::AudioDevice(QObject *parent, QString defaultDevice, bool showAllInputs) : parent(parent), m_showAllInputs(showAllInputs)
{

    setupDevice(defaultDevice);

    // initialize data structures
    m_wholeTrackData = new AudioData;
    m_shortDataBuffer = new std::list<AudioData>(RING_BUFFER_SIZE, AudioData());
    m_shortData = new AudioData;

}

AudioDevice::~AudioDevice()
{
    delete m_wholeTrackData;
    delete m_shortDataBuffer;
    delete m_shortData;
    delete m_audioIn;
}

void AudioDevice::setupDevice(QString deviceName)
{
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    for (auto it = devices.begin(); it != devices.end(); it++) {
        if (it->deviceName().compare(deviceName) == 0) {
            m_monitors.push_front(*it);
        } else if (m_showAllInputs || it->deviceName().endsWith("monitor")) {
            m_monitors.append(*it);
        }
    }

    if (m_monitors.count() == 0) {
        qWarning("No device preferred by settings.JSON or any other sound monitors found.");
        qWarning("Using system default audio input (most likely a microphone)");
        m_monitors.append(QAudioDeviceInfo::defaultInputDevice());
    }

    m_device = m_monitors[0];

    qDebug("Using audio input: %s", qPrintable(m_device.deviceName()));

    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleSize(8);
    m_format.setSampleType(QAudioFormat::UnSignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    QAudioDeviceInfo devInfo(m_device);
    if (!devInfo.isFormatSupported(m_format)) {
        qWarning("Default audio format not supported - trying to use nearest.");
        qWarning("Requested format:");
        qWarning() << m_format;
        m_format = devInfo.nearestFormat(m_format);
        qWarning("Format in use:");
        qWarning() << m_format;
    }

    m_audioIn = new QAudioInput(m_device, m_format, this);
    m_audioIn->setNotifyInterval(1000); // 1 second

    connect(m_audioIn, &QAudioInput::notify, this, &AudioDevice::processAudioIn);
    m_inputBuffer.open(QBuffer::ReadWrite);
    m_audioIn->start(&m_inputBuffer);
}

QStringList AudioDevice::getAudioDevices()
{
    QStringList deviceNames;
    for (auto it = m_monitors.begin(); it != m_monitors.end(); it++) {
        deviceNames.append(it->deviceName());
    }

    return deviceNames;
}

void AudioDevice::changeAudioInput(QString deviceName)
{
    m_monitors.clear();
    delete m_audioIn;
    m_wholeTrackData->clear();

    setupDevice(deviceName);
    emit deviceChanged();
}

void AudioDevice::emitAndClearSongBuffer()
{
    emit songDataReady(*m_wholeTrackData, MyTypes::track);
    m_wholeTrackData->clear();
}

void AudioDevice::printStateChange(QAudio::State state)
{
    qDebug("audiodevice state changed to: %d", state);
}

void AudioDevice::processAudioIn()
{
    m_inputBuffer.seek(0);
    QByteArray ba = m_inputBuffer.readAll();

    m_inputBuffer.buffer().clear();
    m_inputBuffer.seek(0);

    updateShortBuffer(ba);
    updateSongBuffer(ba);

}

void AudioDevice::updateShortBuffer(const QByteArray &data)
{
    m_shortDataBuffer->pop_front();
    m_shortDataBuffer->push_back(AudioData(data.begin(), data.end()));

    m_shortData->clear();
    for (auto it = m_shortDataBuffer->begin(); it != m_shortDataBuffer->end(); it++) {
        m_shortData->insert(m_shortData->end(), it->begin(), it->end());
    }

    emit dataReady(*m_shortData, MyTypes::rolling);
}

void AudioDevice::updateSongBuffer(const QByteArray &data)
{
    m_wholeTrackData->insert(m_wholeTrackData->end(), data.begin(), data.end());
}