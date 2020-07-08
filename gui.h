#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "audiodevice.h"
#include "rhythmextractor.h"

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPalette>
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <vector>

class VulkanWindow;

class MainWindow : public QWidget
{
public:
    MainWindow(VulkanWindow *vulkanWindow);

    void manualUpdateTempo();
    void updateLockCheckbox();

    void setTempoLimited();

public slots:
    void updateTempo();
    void autoUpdateTempo(std::pair<float, float> tempoPair);
    void calculateTempo(std::vector<uint8_t> audioData);

    void updateLowerTempoLimit();
    void updateUpperTempoLimit();

    void readSettings();
    void saveSettings();

private:
    VulkanWindow *m_window;

    QLineEdit *setBpmLine;
    QPalette setBpmLinePalette;
    QCheckBox *lockCheckBox;
    QCheckBox *limitCheckBox;
    QLineEdit *lowerBpmLine;
    QLineEdit *upperBpmLine;
    QComboBox *audioSelect;

    float m_tempoLowerLimit = 20.0;
    float m_tempoUpperLimit = 300.0;

    float m_tempo = 60.0;
    float m_tempoLimited = m_tempo;

    QString m_device;
    AudioDevice *m_audio;

    RhythmExtractor *m_rhythm;
};

#endif