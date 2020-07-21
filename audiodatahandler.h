#ifndef AUDIODATAHANDLER_H
#define AUDIODATAHANDLER_H

#include <QObject>
#include <QIODevice>
#include <QAudioFormat>
#include <QByteArray>
#include <vector>

class AudioDataHandler : public QIODevice
{
    Q_OBJECT

public:
    AudioDataHandler(QAudioFormat format);
    void start();
    void stop();

public slots:
    qint64 writeData(const char *newData, qint64 length) override;
    qint64 readData(char *data, qint64 maxSize) override;

signals:
    void dataReady(std::vector<uint8_t> data);

private:
    QAudioFormat m_format;
    QByteArray *m_buffer;
    std::vector<uint8_t> m_data;

    const int shortBufferSize = 350000;
};

#endif