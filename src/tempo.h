#ifndef TEMPO_H
#define TEMPO_H

#include <QtCore>

#include "types.h"


class Tempo : public QObject {
    Q_OBJECT
public:
    Tempo(QObject* parent);

    bool getEnableManualTempo() const { return m_enableManualTempo; }
    double getTempo() const { return m_enableTempoLimits ? m_tempoLimit : m_tempoNoLimit; }
    double getTempoUpperLimit() const { return m_tempoUpperLimit; }
    double getTempoLowerLimit() const { return m_tempoLowerLimit; }
    double getConfidenceThreshold() const { return m_confidenceThreshold; }
    bool getEnableTempoLimits() const { return m_enableTempoLimits; }

public slots:
    void setEnableManualTempo(bool enable) { m_enableManualTempo = enable; }
    //void setTempo(double tempo);
    //void setTempo();
    void setTempoManual(double tempo);
    void setTempoAutomatic(const MyTypes::TempoData& data);
    void setTempoSmooth(double tempo);
    void setConfidenceThreshold(double threshold) { m_confidenceThreshold = threshold; }
    void setTempoUpperLimit(double limit);
    void setTempoLowerLimit(double limit);
    void setEnableTempoLimits(bool enable);
    void smoothenTempo();
    void disableAutomaticTempo(bool disable) { m_disableAutomaticTempo = disable; }

    void setSmootheningTimerInterval(int msec) { m_smootheningTimer->setInterval(msec); }

signals:
    void tempoChanged(double tempo);
    void tempoUpperLimitChanged(double limit);
    void tempoLowerLimitChanged(double limit);

private:
    QObject* m_parent;
    QTimer* m_smootheningTimer;
    bool m_enableManualTempo = false;
    bool m_disableAutomaticTempo = false;
    double m_confidenceThreshold = 3.0;
    double m_tempoTargetLimit = m_tempoLimit;
    double m_tempoTargetNoLimit = m_tempoLimit;
    double m_tempoLimit = 60.0;
    double m_tempoNoLimit = m_tempoLimit;
    double m_tempoSmootheningStep;
    std::list<double> m_tempoBufferLimit { std::list<double>(5, m_tempoLimit) };
    std::list<double> m_tempoBufferNoLimit { std::list<double>(5, m_tempoNoLimit) };

    bool m_enableTempoLimits = false;
    double m_tempoUpperLimit = 300;
    double m_tempoLowerLimit = 1;
    double limitTempo(double tempo) const;
    double filterDoubleHalf(double tempo, const std::list<double>& buffer) const;
    double getBufferAverage(const std::list<double>& buffer) const;
    void emitTempo();
    void emitTempo(double limited, double notLimited);
};


#endif