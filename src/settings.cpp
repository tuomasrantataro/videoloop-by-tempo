#include "settings.h"

void Settings::readSettings(QString fileName)
{
    QJsonObject values;
    QJsonDocument loadDoc;

    QFile loadFile(fileName);

    if (loadFile.open(QIODevice::ReadOnly)) {
        QString settingsData = loadFile.readAll();
        loadDoc = QJsonDocument::fromJson(settingsData.toUtf8());
    }

    if (loadFile.exists() && loadDoc.isObject()) {
        values = loadDoc.object();
    }


    // Audio device for sound monitoring
    if (!values.value("audio_device").isUndefined()) {
        m_audioDevice = values["audio_device"].toString();
    }


    // Enable tempo limiting
    if (!values.value("limit_tempo").isUndefined()) {
        m_limitTempo = values["limit_tempo"].toBool(m_limitTempo);
    }

    // Enable double/half tempo filtering
    //m_filterDouble = false;
    //if (!values.value("filter_double").isUndefined()) {
    //    m_filterDouble = values["filter_double"].toBool();
    //}
    //m_filterDouble = false;


    // Tempo lower limit
    if (!values.value("tempo_lower_limit").isUndefined()) {
        m_tempoLowerLimit = values["tempo_lower_limit"].toDouble(m_tempoLowerLimit);
    }
    if (m_tempoLowerLimit < 1.0) {
        m_tempoLowerLimit = 1.0;
    }

    // Tempo upper limit
    if (!values.value("tempo_upper_limit").isUndefined()) {
        m_tempoUpperLimit = values["tempo_upper_limit"].toDouble(m_tempoUpperLimit);
    }
    if (m_tempoUpperLimit < 2*m_tempoLowerLimit) {
        m_tempoUpperLimit = 2*m_tempoLowerLimit;
    }


    // Screen for showing the video
    if (!values.value("screen").isUndefined()) {
        m_defaultScreenNumber = values["screen"].toInt(m_defaultScreenNumber);
    }
    if (m_defaultScreenNumber < 0) {
        m_defaultScreenNumber = 0;
    }


    // Start as fullscreen
    if (!values.value("start_as_fullscreen").isUndefined()) {
        m_startAsFullscreen = values["start_as_fullscreen"].toBool(m_startAsFullscreen);
    }


    // Show controls when in fullscreen
    if (!values.value("show_tempo_controls").isUndefined()) {
        m_showTempoControlsFullscreen = values["show_tempo_controls"].toBool(m_showTempoControlsFullscreen);
    }


    // Default video loop
    if (!values.value("video_name").isUndefined()) {
        m_videoLoopName = values["video_name"].toString();
    }


    // Confifence level
    if (!values.value("confidence_threshold").isUndefined()) {
        m_confidenceThreshold = values["confidence_threshold"].toDouble(m_confidenceThreshold);
    }
    if (m_confidenceThreshold < 1.0) {
        m_confidenceThreshold = 1.0;
    } else if (m_confidenceThreshold > 5.0) {
        m_confidenceThreshold = 5.0;
    }


    // Player application in use
    if (!values.value("pa_application_name").isUndefined()) {
        m_pulseAudioAppName = values["pa_application_name"].toString();
    }

    if (!values.value("pa_ignore_applications").isUndefined()) {
        QJsonArray ignoreApps = values["pa_ignore_applications"].toArray();
        for (auto item : ignoreApps) {
            m_pulseAudioIgnoreList.push_back(item.toString());
        }
    }


    // Setttings for individual video loops
    QJsonObject loopSettings;
    if (!values.value("loop_settings").isUndefined()) {
        loopSettings = values["loop_settings"].toObject();
    }

    m_videoLoopSettings = loopSettings;
}

void Settings::writeSettings(QString fileName)
{
    QFile saveFile(fileName);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        return;
    }

    QJsonObject settingsData;
    settingsData["audio_device"] = m_audioDevice;
    settingsData["limit_tempo"] = m_limitTempo;
    //settingsData["filter_double"] = m_filterCheckBox->isChecked();
    settingsData["tempo_lower_limit"] = m_tempoLowerLimit;
    settingsData["tempo_upper_limit"] = m_tempoUpperLimit;
    settingsData["screen"] = m_defaultScreenNumber;
    settingsData["start_as_fullscreen"] = m_startAsFullscreen;
    settingsData["show_tempo_controls"] = m_showTempoControlsFullscreen;
    settingsData["video_name"] = m_videoLoopName;
    settingsData["confidence_threshold"] = m_confidenceThreshold;
    settingsData["pa_application_name"] = m_pulseAudioAppName;
    settingsData["pa_ignore_applications"] = QJsonArray::fromStringList(m_pulseAudioIgnoreList);
    settingsData["loop_settings"] = m_videoLoopSettings;   

    saveFile.write(QJsonDocument(settingsData).toJson());
}

bool Settings::getLoopAddReversedFrames(QString loopName) const
{
    QJsonObject loopData = m_videoLoopSettings[loopName].toObject();
    bool defaultValue = false;
    if (loopData.value("add_reversed_frames").isUndefined()) {
        return defaultValue;
    }
    
    return loopData["add_reversed_frames"].toBool(defaultValue);
}

double Settings::getLoopTempoMultiplier(QString loopName) const
{
    QJsonObject loopData = m_videoLoopSettings[loopName].toObject();
    double defaultValue = 1.0;
    if (loopData.value("tempo_multiplier").isUndefined()) {
        return defaultValue;
    }
    
    if (loopData.value("tempo_multiplier").toDouble() < 0.01) {
        return defaultValue;
    }
    
    return loopData["tempo_multiplier"].toDouble(defaultValue);
}

void Settings::setLoopAddReversedFrames(QString loopName, bool addReversedFrames)
{
    QJsonObject settingsData;
    settingsData["add_reversed_frames"] = addReversedFrames;
    settingsData["tempo_multiplier"] = getCurrentLoopTempoMultiplier();

    m_videoLoopSettings[loopName] = settingsData;
}
void Settings::setCurrentLoopAddReversedFrames(bool addReversedFrames)
{
    setLoopAddReversedFrames(m_videoLoopName, addReversedFrames);
}

void Settings::setLoopTempoMultiplier(QString loopName, double tempoMultiplier)
{
    QJsonObject settingsData;
    settingsData["add_reversed_frames"] = getCurrentLoopAddReversedFrames();
    settingsData["tempo_multiplier"] = tempoMultiplier;

    m_videoLoopSettings[loopName] = settingsData;

}
void Settings::setCurrentLoopTempoMultiplier(double tempoMultiplier)
{
    setLoopTempoMultiplier(m_videoLoopName, tempoMultiplier);
}
