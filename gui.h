#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "audiodevice.h"
#include "rhythmextractor.h"
#include "openglwidget.h"

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPalette>
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QShortcut>
#include <QCloseEvent>
#include <vector>
#include <list>

class VulkanWindow;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void updateTempo();

    void manualUpdateTempo();
    void toggleManualTempo();
    void updateLockCheckbox();

    void autoUpdateTempo(std::pair<float, float> tempoPair);
    void calculateTempo(std::vector<uint8_t> audioData);
    void setConfidenceLevel(int value);

    void setTempoMultiplier(int value);

    void readLowerLimit();
    void readUpperLimit();

    void setVideoFullScreen();
    void setScreenNumber(int idx);

    void setStartFullScreen();
    void setShowTempoControls();

    void saveSettings();

    void fixSize();

private:
    void setTempoLimited();
    void updateLowerTempoLimit(float limit);
    void updateUpperTempoLimit(float limit);
    void readSettings();

    OpenGLWidget *m_graphicsWidget;

    QVBoxLayout *m_layout;

    QGroupBox *m_tempoGroup;
    QGroupBox *m_limitGroup;
    QGroupBox *m_audioGroup;
    QGroupBox *m_videoGroup;

    QLineEdit *m_setBpmLine;
    QPalette m_setBpmLinePalette;
    QCheckBox *m_lockCheckBox;
    bool m_lockTempo;
    QCheckBox *m_limitCheckBox;
    bool m_limitTempo;
    QLineEdit *m_lowerBpmLine;
    QLineEdit *m_upperBpmLine;
    QCheckBox *m_startFullScreenCheckBox;
    QComboBox *m_screenSelect;
    QList<QScreen*> m_screens;
    int m_screenNumber;
    bool m_startAsFullScreen;
    QCheckBox *m_tempoControlsCheckBox;
    bool m_showTempoControls;

    QComboBox *m_audioSelect;

    QSlider *m_confidenceSlider;
    float m_confidenceLevel;

    QLabel *m_tempoMultiplierLabel;
    float m_tempoMultiplier;
    QSlider *m_tempoMultiplierSlider;

    float m_tempoLowerLimit;
    float m_tempoUpperLimit;

    float m_tempo = 60.0;
    float m_tempoLimited = m_tempo;

    std::list<float> m_bpmBuffer{std::list<float>(5, m_tempo)};

    QString m_device;
    AudioDevice *m_audio;

    RhythmExtractor *m_rhythm;

    QShortcut *m_keySpacebar;
};

#endif