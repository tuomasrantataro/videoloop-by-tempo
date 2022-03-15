#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QShortcut>
#include <QPushButton>
#include <QCommandLineParser>

#include "audiodevice.h"
#include "rhythmextractor.h"
#include "types.h"
#include "settings.h"
#include "tempo.h"
#include "track.h"
#include "screensaverinhibitor.h"


#include "graphicswidget.h"

using namespace MyTypes;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QCommandLineParser *parser);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void updateTempo(double tempo);

    void toggleManualTempo();
    void updateLockCheckBox();

    void setTempoMultiplier(int value);

    void readLowerLimit();
    void readUpperLimit();

    void setVideoFullScreen();
    void setScreenNumber(int idx);

    void setStartFullScreen();
    void setShowTempoControls();

    void setAddReversedFrames();
    void setVideoLoop(QString loopName);

    void fixSize();

    void receiveBPMCalculationResult(const TempoData& data, MyTypes::AudioBufferType type);

    void bpmLineChanged();

    void setConfidenceThreshold(int value);

    void setLowerTempoLimit(double limit);
    void setUpperTempoLimit(double limit);

private:
    void initUI();

    // UI members
    GraphicsWidget *m_graphics;

    QVBoxLayout *m_layout;
    QGridLayout *m_videoLayout;

    QGroupBox *m_tempoGroup;
    QGroupBox *m_limitGroup;
    QGroupBox *m_audioGroup;
    QGroupBox *m_videoGroup;

    QLineEdit *m_setBpmLine;
    QPalette m_setBpmLinePalette;
    QCheckBox *m_lockCheckBox;
    QPushButton *m_wrongTempoButton;
    QSlider *m_confidenceSlider;
    QSlider *m_thresholdSlider;

    QCheckBox *m_limitCheckBox;
    QLineEdit *m_lowerBpmLine;
    QLineEdit *m_upperBpmLine;
    QLabel *m_tempoMultiplierLabel;
    QSlider *m_tempoMultiplierSlider;
    
    QComboBox *m_audioSelect;
    QComboBox *m_loopSelect;
    QCheckBox *m_startFullScreenCheckBox;
    QCheckBox *m_tempoControlsCheckBox;
    QCheckBox *m_reverseFramesCheckBox;
    QComboBox *m_screenSelect;
    QList<QScreen*> m_screens;
    
    QShortcut *m_keySpacebar;

    QCommandLineParser *m_parser;

    // Non-UI members
    Settings *m_settings;

    AudioDevice *m_audio;
    RhythmExtractor *m_rhythm;

    Tempo *m_tempoHandler;
    Track *m_trackHandler;  

    double m_currentTempo;

    ScreenSaverInhibitor *m_screenSaverInhibitor;

};

#endif
