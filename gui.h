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
#include <vector>

class VulkanWindow;

class MainWindow : public QWidget
{
public:
    MainWindow(VulkanWindow *vulkanWindow);

    void manualUpdateTempo();
    void updateTempo(float bpm);
    void updateLockCheckbox();

public slots:
    void autoUpdateTempo(std::pair<float, float> tempoPair);
    void calculateTempo(std::vector<uint8_t> audioData);

private:
    VulkanWindow *m_window;

    QLineEdit *setBpmLine;
    QPalette setBpmLinePalette;
    QCheckBox *lockCheckBox;
    QCheckBox *limitCheckBox;
    QComboBox *audioSelect;

    float tempoLowerLimit = 60.0;
    float tempoUpperLimit = 120.0;

    float m_tempo = 60.0;

    QString defaultDevice = QString("alsa_output.pci-0000_00_1f.3.analog-stereo.monitor");
    AudioDevice *m_audio;

    RhythmExtractor *m_rhythm;
};

#endif