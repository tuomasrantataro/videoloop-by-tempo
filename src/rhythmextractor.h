#ifndef RHYTHMEXTRACTOR_H
#define RHYTHMEXTRACTOR_H

#include <essentia/pool.h>
#include <essentia/algorithmfactory.h>

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <vector>

#include "types.h"

using namespace MyTypes;

class RhythmWorker;

class RhythmExtractor : public QObject
{
    Q_OBJECT
public:
    RhythmExtractor(QObject* parent);

    ~RhythmExtractor();

signals:
    void calculateTempo(const std::vector<uint8_t>& audioData, const AudioBufferType type);
    void calculationReady(const TempoData& result, const AudioBufferType type);

private:
    QObject *m_parent;

    RhythmWorker *m_worker;
    QThread m_workerThread;
};



class RhythmWorker : public QObject
{
    Q_OBJECT

public slots:
    void calculateBPM(const std::vector<uint8_t>& audioData, const AudioBufferType type);

signals:
    void resultReady(const TempoData& result, const AudioBufferType type);
};

#endif