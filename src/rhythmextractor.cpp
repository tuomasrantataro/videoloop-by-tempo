#include "rhythmextractor.h"

RhythmExtractor::RhythmExtractor(QObject *parent) : m_parent(parent)
{
    m_worker = new RhythmWorker;
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &RhythmExtractor::calculateTempo, m_worker, &RhythmWorker::calculateBPM);
    connect(m_worker, &RhythmWorker::resultReady, this, &RhythmExtractor::calculationReady);
    m_workerThread.start();
}

RhythmExtractor::~RhythmExtractor()
{
    m_workerThread.quit();
    m_workerThread.wait();
}



void RhythmWorker::calculateBPM(const AudioData& audioData, const AudioBufferType type)
{
    using namespace essentia;

    if (audioData.begin() == audioData.end()) {
        // Do nothing if there's no data
        return;
    }


    std::vector<essentia::Real> data = std::vector<essentia::Real>(audioData.begin(), audioData.end());

    essentia::init();

    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();

    standard::Algorithm* rhythm = factory.create("RhythmExtractor2013",
                                                 "method", "multifeature");

    standard::Algorithm* histo = factory.create("BpmHistogramDescriptors");

    rhythm->input("signal").set(data);

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

    TempoData ret = {ticks,
                     confidence,
                     bpm,
                     firstPeak,
                     firstPeakWeight,
                     firstPeakSpread,
                     secondPeak,
                     secondPeakWeight,
                     secondPeakSpread,
                     bpmEstimates,
                     bpmIntervals};

    emit resultReady(ret, type);

    delete rhythm;
    
    essentia::shutdown();
}
