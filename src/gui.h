#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QShortcut>
#include <QCloseEvent>
#include <QPushButton>
#include <vector>
#include <list>
#include <QCommandLineParser>

#include "audiodevice.h"
#include "rhythmextractor.h"
#include "openglwidget.h"
#include "spotifywatcher.h"
#include "pulseaudiowatcher.h"
#include "dbmanager.h"
#include "types.h"
#include "settings.h"
#include "tempo.h"

using namespace MyTypes;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QCommandLineParser *parser);
    ~MainWindow();

    int checkInit();

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

    void saveTrackData();

    void updateTrackInfo(QString oldTrackId, QString oldArtist, QString oldTitle, QString newTrackId);
    void updateTrackTempo(const TempoData& data);

    void receiveBPMCalculationResult(const TempoData& data, MyTypes::AudioBufferType type);

    void invalidateTrackData(QString reason);

    void removeCurrentTrack();

    void bpmLineChanged();

    void setConfidenceThreshold(int value);

    void setLowerTempoLimit(double limit);
    void setUpperTempoLimit(double limit);

private:
    Settings* m_settings;

    int checkDirectories();

    QCommandLineParser *m_parser;

    OpenGLWidget *m_graphicsWidget;

    QVBoxLayout *m_layout;
    QGridLayout *m_videoLayout;

    QGroupBox *m_tempoGroup;
    QGroupBox *m_limitGroup;
    QGroupBox *m_audioGroup;
    QGroupBox *m_videoGroup;

    QLineEdit *m_setBpmLine;
    QPalette m_setBpmLinePalette;
    QCheckBox *m_lockCheckBox;
    QCheckBox *m_filterCheckBox;

    QCheckBox *m_limitCheckBox;
    QLineEdit *m_lowerBpmLine;
    QLineEdit *m_upperBpmLine;
    QCheckBox *m_startFullScreenCheckBox;
    QComboBox *m_screenSelect;
    QList<QScreen*> m_screens;
    QCheckBox *m_tempoControlsCheckBox;

    QCheckBox *m_reverseFramesCheckBox;

    QComboBox *m_loopSelect;

    QComboBox *m_audioSelect;

    QSlider *m_confidenceSlider;
    QSlider *m_thresholdSlider;

    QLabel *m_tempoMultiplierLabel;
    QSlider *m_tempoMultiplierSlider;

    TrackData m_trackData;

    bool m_invalidTrackData = false;

    bool m_disableAutoTempo = false;

    QTimer *m_smoothTempoUpdateTimer;

    QStringList m_trackDataInvalidationReasons;

    QString m_currentTrackId;

    Tempo *m_tempoHandler;

    AudioDevice *m_audio;

    RhythmExtractor *m_rhythm;

    QShortcut *m_keySpacebar;

    int m_initError = 0;

    SpotifyWatcher *m_spotifyWatcher;

    PulseaudioWatcher *m_pulseaudioWatcher;

    DBManager *m_trackDBManager;

    QPushButton *m_wrongTempoButton;

signals:
    void trackCalculationNeeded();

};

#endif
