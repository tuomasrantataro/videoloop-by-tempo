#include "tempo.h"

Tempo::Tempo(QObject* parent) : m_parent(parent)
{
    m_smootheningTimer = new QTimer(m_parent);
    m_smootheningTimer->setInterval(1000);
    connect(m_smootheningTimer, &QTimer::timeout, this, &Tempo::smoothenTempo);
}

void Tempo::emitTempo()
{
    if (m_enableTempoLimits) {
        emit tempoChanged(m_tempoLimit);
    }
    else {
        emit tempoChanged(m_tempoNoLimit);
    }
}

void Tempo::emitTempo(double limited, double notLimited)
{
    m_tempoLimit = limited;
    m_tempoNoLimit = notLimited;
    
    emitTempo();
}

void Tempo::setTempoManual(double tempo)
{
    m_tempoTargetNoLimit = tempo;
    m_tempoBufferNoLimit.assign(5, m_tempoTargetNoLimit);

    m_tempoTargetLimit = limitTempo(tempo);
    m_tempoBufferLimit.assign(5, m_tempoTargetLimit);

    emitTempo(limitTempo(tempo), tempo);
}

void Tempo::setTempoAutomatic(const MyTypes::TempoData& data)
{
    double tempo_ = data.BPM;
    double confidence_ = data.confidence;

    if (confidence_ < m_confidenceThreshold) {
        return;
    }

    // filter double/half tempos
    tempo_ = filterDoubleHalf(tempo_, m_tempoBufferNoLimit);

    m_tempoBufferLimit.pop_front();
    m_tempoBufferLimit.push_back(limitTempo(tempo_));

    m_tempoBufferNoLimit.pop_front();
    m_tempoBufferNoLimit.push_back(tempo_);

    if (m_enableManualTempo || m_disableAutomaticTempo) {
        return;
    }
    
    m_tempoTargetLimit = limitTempo(tempo_);
    m_tempoTargetNoLimit = tempo_;

    emitTempo(getBufferAverage(m_tempoBufferLimit), getBufferAverage(m_tempoBufferNoLimit));
}

void Tempo::setTempoSmooth(double tempo)
{
    m_tempoTargetNoLimit = tempo;
    m_tempoTargetLimit = limitTempo(tempo);

    if (m_enableTempoLimits) {
        m_tempoSmootheningStep = (m_tempoLimit - m_tempoTargetLimit)/7.0;
    }
    else {
        m_tempoSmootheningStep = (m_tempoNoLimit - m_tempoTargetNoLimit)/7.0;
    }
    
    m_smootheningTimer->start();
}

void Tempo::smoothenTempo()
{
    double tempo = m_tempoNoLimit;
    double target = m_tempoTargetNoLimit;

    if (m_enableTempoLimits) {
        tempo = m_tempoLimit;
        target = m_tempoTargetLimit;
    }

    if (abs(tempo - target) < abs(m_tempoSmootheningStep)) {
        tempo = target;
        m_smootheningTimer->stop();
    }
    else {
        tempo = tempo - m_tempoSmootheningStep;
    }

    if (!m_enableManualTempo) {
        emitTempo(limitTempo(tempo), tempo);
    }
}

double Tempo::filterDoubleHalf(double tempo, const std::list<double>& buffer) const
{
    double tempo_ = tempo;
    double oldTempo = getBufferAverage(buffer);
    double threshold = 0.1;

    if (abs(tempo_ - 2.0*oldTempo) < threshold*tempo_) {
        // fix double tempo
        tempo_ = tempo_/2.0;
    } else if (abs(tempo_ - 0.5*oldTempo) < threshold*tempo_) {
        // fix half tempo
        tempo_ = tempo_*2.0;
    }

    return tempo_;
}

double Tempo::getBufferAverage(const std::list<double>& buffer) const
{
    double average = 0.0;
    for (auto item : buffer) {
        average += item;
    }
    average = average/buffer.size();

    return average;
}

double Tempo::limitTempo(double tempo) const
{
    double tempo_ = tempo;
    if (tempo_ < 1.0) {  // prevent infinite loop multiplying 0
        tempo_ = 1.0;
    }

    while (tempo_ < m_tempoLowerLimit) {
        tempo_ = tempo_ * 2.0;
    }
    while (tempo_ > m_tempoUpperLimit) {
        tempo_ = tempo_ / 2.0;
    }

    return tempo_;
}


void Tempo::setEnableTempoLimits(bool enable)
{
    m_enableTempoLimits = enable;

    emitTempo(limitTempo(m_tempoNoLimit), m_tempoNoLimit);
}

void Tempo::setTempoLowerLimit(double limit)
{
    double upper = m_tempoUpperLimit;
    if (limit > 0.0) {
        // The upper limit must be at least twice the lower limit
        if (limit > upper/2.0) {
            limit = upper/2.0;
        }
        m_tempoLowerLimit = limit;
        emit tempoLowerLimitChanged(limit);
    }

    // Check limits with new values
    setEnableTempoLimits(m_enableTempoLimits);
}

void Tempo::setTempoUpperLimit(double limit)
{
    double lower = m_tempoLowerLimit;
    if (limit > 1.0 && limit < 300.0) {
        // The upper limit must be at least twice the lower limit
        if (limit < lower*2.0) {
            limit = lower*2.0;
        }

        m_tempoUpperLimit = limit;
        emit tempoUpperLimitChanged(limit);
    }

    // Check limits with new values
    setEnableTempoLimits(m_enableTempoLimits);
}
