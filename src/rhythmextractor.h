#ifndef RHYTHMEXTRACTOR_H
#define RHYTHMEXTRACTOR_H

#include <essentia/pool.h>
#include <essentia/algorithmfactory.h>
#include <essentia/streaming/streamingalgorithm.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/streaming/algorithms/vectorinput.h>
#include <essentia/scheduler/network.h>

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <vector>

typedef std::pair<float, float> tempoPair;
Q_DECLARE_METATYPE(tempoPair)


class RhythmExtractor : public QThread
{
    Q_OBJECT

public:
    RhythmExtractor(QObject *parent = nullptr);
    ~RhythmExtractor();

    void calculateTempo(std::vector<uint8_t> audioData);

signals:
    void tempoReady(tempoPair);

protected:
    void run() override;

private:
    std::vector<essentia::Real> data;
    
    QMutex mutex;
    QWaitCondition condition;
    bool restart = false;
    bool abort = false;
};

#endif