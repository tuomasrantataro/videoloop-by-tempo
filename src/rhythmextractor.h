#ifndef RHYTHMEXTRACTOR_H
#define RHYTHMEXTRACTOR_H

#include <essentia/pool.h>
#include <essentia/algorithmfactory.h>
//#include <essentia/streaming/streamingalgorithm.h>
//#include <essentia/streaming/algorithms/poolstorage.h>
//#include <essentia/streaming/algorithms/vectorinput.h>
//#include <essentia/standard/streamingalgorithm.h>
//#include <essentia/streaming/algorithms/poolstorage.h>
//#include <essentia/streaming/algorithms/vectorinput.h>
//#include <essentia/scheduler/network.h>

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <vector>

typedef std::pair<float, float> tempoPair;
Q_DECLARE_METATYPE(tempoPair)

typedef struct TempoData {
    std::vector<float> ticks; // beat ticks, [s] from data start
    float confidence; // confidence in beat detection. >3.5 is excellent
    float BPM;  // bpm estimate
    std::vector<float> BPMEstimates; // array of bpm estimates starting from most likely
    std::vector<float> BPMIntervals; // list of beats intervals [s]
} TempoData;
Q_DECLARE_METATYPE(TempoData)


class RhythmExtractor : public QThread
{
    Q_OBJECT

public:
    RhythmExtractor(QObject *parent = nullptr);
    ~RhythmExtractor();

public slots:
    void calculateTempo(const std::vector<uint8_t>& audioData);

signals:
    void tempoReady(tempoPair);
    void calculationReady(TempoData);

protected:
    void run() override;

private:
    std::vector<essentia::Real> m_data;
    
    QMutex mutex;
    QWaitCondition condition;
    bool restart = false;
    bool abort = false;
};

#endif