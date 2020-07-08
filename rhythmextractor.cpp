
#include "rhythmextractor.h"
#include <vector>

RhythmExtractor::RhythmExtractor(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<tempoPair>("TempoPair");
}

RhythmExtractor::~RhythmExtractor()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void RhythmExtractor::calculateTempo(std::vector<uint8_t> audioData)
{
    QMutexLocker locker(&mutex);

    this->data = std::vector<essentia::Real>(audioData.begin(), audioData.end());

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}

void RhythmExtractor::run()
{
    essentia::init();

    mutex.lock();
    const std::vector<essentia::Real> audio = this->data;
    mutex.unlock();

    essentia::Pool pool;

    essentia::streaming::AlgorithmFactory& factory = essentia::streaming::AlgorithmFactory::instance();

    essentia::streaming::Algorithm* rhythmextractor = factory.create("RhythmExtractor2013",
                                                                     "method", "multifeature");

    essentia::streaming::VectorInput<essentia::Real, 1> input(&audio);
    input.setAcquireSize(audio.size());
    input.configure();

    input.output("data")                    >> rhythmextractor->input("signal");
    rhythmextractor->output("ticks")        >> PC(pool, "rhythm.ticks");
    rhythmextractor->output("confidence")   >> PC(pool, "rhythm.ticks_confidence");
    rhythmextractor->output("bpm")          >> PC(pool, "rhythm.bpm");
    rhythmextractor->output("estimates")    >> PC(pool, "rhythm.estimates");
    rhythmextractor->output("bpmIntervals") >> PC(pool, "rhythm.bpmIntervals");

    essentia::scheduler::Network network(&input, false);
    network.run();

    float bpm = float(pool.value<essentia::Real>("rhythm.bpm"));
    float confidence = float(pool.value<essentia::Real>("rhythm.ticks_confidence"));

    emit tempoReady(tempoPair(bpm, confidence));

    delete rhythmextractor;
    essentia::shutdown();
}