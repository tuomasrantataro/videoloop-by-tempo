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
    void updateTempo();

    void smoothUpdateTempo();

    void manualUpdateTempo();
    void toggleManualTempo();
    void updateLockCheckBox();
    void updateFilterCheckBox();

    void autoUpdateTempo(const TempoData& tempoData);
    void setConfidenceLevel(int value);

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

private:
    Settings* m_settings;

    void setTempoLimited();
    void updateLowerTempoLimit(float limit);
    void updateUpperTempoLimit(float limit);

    bool detectDoubleTempoJump(float newTempo);

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
    bool m_lockTempo;
    QCheckBox *m_filterCheckBox;
    bool m_filterDouble;

    QCheckBox *m_limitCheckBox;
    bool m_limitTempo;
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
    float m_confidenceLevel;
    QSlider *m_thresholdSlider;

    QLabel *m_tempoMultiplierLabel;
    QSlider *m_tempoMultiplierSlider;

    float m_tempo = 60.0;
    float m_tempoLimited = m_tempo;
    float m_targetTempo;
    float m_step;

    std::list<float> m_bpmBuffer{std::list<float>(5, m_tempo)};
    std::list<float> m_rejectedBpmBuffer{std::list<float>(10, 1.0)};

    //QString m_device;
    AudioDevice *m_audio;

    RhythmExtractor *m_rhythm;

    QShortcut *m_keySpacebar;

    int m_initError = 0;

    SpotifyWatcher *m_spotifyWatcher;

    PulseaudioWatcher *m_pulseaudioWatcher;

    DBManager *m_trackDBManager;

    TrackData m_trackData;

    bool m_invalidTrackData = false;

    bool m_disableAutoTempo = false;

    QTimer *m_smoothTempoUpdateTimer;

    QStringList m_trackDataInvalidationReasons;

    QPushButton *m_wrongTempoButton;

    QString m_currentTrackId;

signals:
    void trackCalculationNeeded();

};

#endif
