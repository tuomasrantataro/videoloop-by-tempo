
#include "audiodatahandler.h"

AudioDataHandler::AudioDataHandler(QAudioFormat format) : m_format(format)
{
    m_buffer = new QByteArray();
}

void AudioDataHandler::start()
{
    open(QIODevice::WriteOnly);
}

void AudioDataHandler::stop()
{
    close();
}

qint64 AudioDataHandler::writeData(const char *newData, qint64 length)
{
    QByteArray data = QByteArray(newData);
    m_buffer->append(data.data());

    m_buffer->replace(0, shortBufferSize, m_buffer->right(shortBufferSize));
    m_buffer->truncate(shortBufferSize);
    const unsigned char* begin = reinterpret_cast<unsigned char*>(m_buffer->data());
    const unsigned char* end = begin + m_buffer->length();
    m_data.assign(begin, end);

    emit dataReady(m_data);
    return length;
}

qint64 AudioDataHandler::readData(char* data, qint64 maxSize)
{
    return maxSize;
}