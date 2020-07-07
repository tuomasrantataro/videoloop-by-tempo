#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "audiodevice.h"
#include "rhythmextractor.h"

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPalette>
#include <QString>
#include <vector>

class VulkanWindow;

class MainWindow : public QWidget
{
public:
    MainWindow(VulkanWindow *vulkanWindow);

    void updateBpmManually();
    void updateBpm(float bpm, bool manual=false);
    void changePlayBackRate(float);
    void updateLockCheckbox();

public slots:
    void autoUpdateBpm(std::pair<float, float> tempo);
    void calculateBPM(std::vector<uint8_t> data);

private:
    VulkanWindow *m_window;

    QLineEdit *setBpmLine;
    QPalette setBpmLinePalette;
    QCheckBox *lockCheckBox;
    QCheckBox *limitCheckBox;
    float tempoLowerLimit = 60.0;
    float tempoUpperLimit = 120.0;
    float oldBpm = 60.0;

    QString defaultDevice = QString("alsa_output.pci-0000_00_1f.3.analog-stereo.monitor");
    AudioDevice *m_audio;

    RhythmExtractor *m_rhythm;
};

#endif