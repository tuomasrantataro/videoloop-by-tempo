#include "rhythmextractor.h"

RhythmExtractor::RhythmExtractor(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<tempoPair>("TempoPair");
    qRegisterMetaType<TempoData>("TempoData");
}

RhythmExtractor::~RhythmExtractor()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void RhythmExtractor::calculateTempo(const std::vector<uint8_t>& audioData)
{
    QMutexLocker locker(&mutex);

    m_data = std::vector<essentia::Real>(audioData.begin(), audioData.end());

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}

void RhythmExtractor::run()
{
    using namespace essentia;

    essentia::init();

    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();

    standard::Algorithm* rhythm = factory.create("RhythmExtractor2013",
                                                 "method", "multifeature");

    standard::Algorithm* histo = factory.create("BpmHistogramDescriptors");

    rhythm->input("signal").set(m_data);

    std::vector<essentia::Real> ticks, bpmEstimates, bpmIntervals;
    essentia::Real confidence, bpm;

    rhythm->output("ticks").set(ticks);
    rhythm->output("confidence").set(confidence);
    rhythm->output("bpm").set(bpm);
    rhythm->output("estimates").set(bpmEstimates);
    rhythm->output("bpmIntervals").set(bpmIntervals);

    rhythm->compute();

    essentia::Real firstPeak, firstPeakWeight, firstPeakSpread;
    essentia::Real secondPeak, secondPeakWeight, secondPeakSpread;
    std::vector<essentia::Real> hist;

    histo->input("bpmIntervals").set(bpmIntervals);
    histo->output("firstPeakBPM").set(firstPeak);
    histo->output("firstPeakWeight").set(firstPeakWeight);
    histo->output("firstPeakSpread").set(firstPeakSpread);
    histo->output("secondPeakBPM").set(secondPeak);
    histo->output("secondPeakWeight").set(secondPeakWeight);
    histo->output("secondPeakSpread").set(secondPeakSpread);
    histo->output("histogram").set(hist);
    histo->compute();
    //qDebug("first peak: %f, w: %f, s: %f", firstPeak, firstPeakWeight, firstPeakSpread);
    //qDebug("second peak: %f, w: %f, s: %f", secondPeak, secondPeakWeight, secondPeakSpread);

    TempoData ret = {ticks, confidence, bpm, bpmEstimates, bpmIntervals};

    emit calculationReady(ret);
    emit tempoReady(tempoPair(ret.BPM, ret.confidence));

    delete rhythm;
    
    essentia::shutdown();
}