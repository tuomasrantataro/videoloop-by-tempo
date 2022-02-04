#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore>

class Settings : QObject
{
    Q_OBJECT

public:
    void readSettings(QString fileName);
    void writeSettings(QString fileName);

    QString getAudioDevice() const { return m_audioDevice; }
    double getConfidenceThreshold() const { return m_confidenceThreshold; }
    bool getLimitTempo() const { return m_limitTempo; }
    QString getPulseAudioAppName() const { return m_pulseAudioAppName; }
    QStringList getPulseAudioIgnoreList() const { return m_pulseAudioIgnoreList; }
    int getScreenNumber() const { return m_defaultScreenNumber; }
    bool getShowTempoControls() const { return m_showTempoControlsFullscreen; }
    bool getStartAsFullscreen() const { return m_startAsFullscreen; }
    double getTempoLowerLimit() const { return m_tempoLowerLimit; }
    double getTempoUpperLimit() const { return m_tempoUpperLimit; }
    QString getVideoLoopName() const { return m_videoLoopName; }

    void setAudioDevice(QString name) { m_audioDevice = name; }
    void setConfidenceThreshold(double threshold) { m_confidenceThreshold = threshold; }
    void setLimitTempo(bool limitTempo) { m_limitTempo = limitTempo; }
    void setPulseAudioAppName(QString name) { m_pulseAudioAppName = name; }
    void setPulseAudioIgnoreList(QStringList names) { m_pulseAudioIgnoreList = names; }
    void setScreenNumber(int screen) { m_defaultScreenNumber = screen; }
    void setShowTempoControls(bool showControls) { m_showTempoControlsFullscreen = showControls; }
    void setStartAsFullscreen(bool fullscreen) { m_startAsFullscreen = fullscreen; }
    void setTempoLowerLimit(double limit) { m_tempoLowerLimit = limit; }
    void setTempoUpperLimit(double limit) { m_tempoUpperLimit = limit; }
    void setVideoLoopName(QString name) { m_videoLoopName = name; }

    bool getLoopAddReversedFrames(QString loopName) const;
    double getLoopTempoMultiplier(QString loopName) const;
    bool getCurrentLoopAddReversedFrames() const { return getLoopAddReversedFrames(m_videoLoopName); }
    double getCurrentLoopTempoMultiplier() const { return getLoopTempoMultiplier(m_videoLoopName); }

    void setLoopAddReversedFrames(QString loopName, bool addReversedFrames);
    void setCurrentLoopAddReversedFrames(bool addReversedFrames);
    void setLoopTempoMultiplier(QString loopName, double tempoMultiplier);
    void setCurrentLoopTempoMultiplier(double tempoMultiplier);

private:
    QString m_audioDevice;
    double m_confidenceThreshold = 3.0;
    bool m_limitTempo = false;
    QString m_pulseAudioAppName;
    QStringList m_pulseAudioIgnoreList;
    int m_defaultScreenNumber = 0;
    bool m_showTempoControlsFullscreen = false;
    bool m_startAsFullscreen = false;
    double m_tempoLowerLimit = 60.0;
    double m_tempoUpperLimit = 120.0;
    QString m_videoLoopName;

    QJsonObject m_videoLoopSettings;
};

#endif