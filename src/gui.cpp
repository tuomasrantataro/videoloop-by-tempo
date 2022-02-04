#include "gui.h"
#include <QPushButton>
#include <QApplication>
#include <QPalette>
#include <QWindow>
#include <QScreen>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QHBoxLayout>
#include <QGridLayout>

using namespace MyTypes;

MainWindow::MainWindow(QCommandLineParser *parser) : m_parser(parser)
{
    
    if (checkDirectories()) {
        // Exit right away if video frames are not found
        return;
    };

    qRegisterMetaType<TempoData>("TempoData");

    //FIXME: why this is needed in addition to AudioData
    qRegisterMetaType<std::vector<uint8_t>>("std::vector<uint8_t>");
    qRegisterMetaType<AudioData>("AudioData");
    qRegisterMetaType<AudioBufferType>("AudioBufferType");
    
    m_settings = new Settings();
    m_settings->readSettings("settings.JSON");

    m_graphicsWidget = new OpenGLWidget(m_settings->getVideoLoopName());
    connect(m_graphicsWidget, &OpenGLWidget::toggleFullScreen, this, &MainWindow::setVideoFullScreen);
    connect(m_graphicsWidget, &OpenGLWidget::initReady, this, &MainWindow::updateTempo);
    connect(m_graphicsWidget, &OpenGLWidget::initReady, this, &MainWindow::setAddReversedFrames);
    connect(m_graphicsWidget, &OpenGLWidget::spacePressed, this, &MainWindow::toggleManualTempo);


    // Layout for setting tempo
    m_tempoGroup = new QGroupBox(tr("Tempo"));

    m_lockCheckBox = new QCheckBox(tr("Manual tempo"), this);
    connect(m_lockCheckBox, &QPushButton::clicked, this, &MainWindow::updateLockCheckBox);

    m_setBpmLine = new QLineEdit(QString::number(m_tempo, 'f', 1));
    m_setBpmLine->setMaxLength(5);
    m_setBpmLine->setMaximumWidth(50);
    m_setBpmLinePalette = QPalette();
    m_setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
    m_setBpmLine->setPalette(m_setBpmLinePalette);
    connect(m_setBpmLine, &QLineEdit::returnPressed, this, &MainWindow::manualUpdateTempo);

    QLabel *bpmLabel = new QLabel(tr("bpm"));
    QHBoxLayout *tempoLine = new QHBoxLayout;
    tempoLine->addWidget(m_setBpmLine);
    tempoLine->addWidget(bpmLabel);

    // FIXME: currently not added to UI, but deleting this causes segfault on program exit
    //m_filterCheckBox = new QCheckBox(tr("Filter half/double tempo"));
    //m_filterCheckBox->setChecked(m_filterDouble);
    //connect(m_filterCheckBox, &QPushButton::clicked, this, &MainWindow::updateFilterCheckBox);
    

    QPushButton *m_wrongTempoButton = new QPushButton(tr("Wrong tempo"));
    connect(m_wrongTempoButton, &QPushButton::clicked, this, &MainWindow::removeCurrentTrack);


    QLabel *confidenceLabel = new QLabel(tr("BPM confidence"));
    m_confidenceSlider = new QSlider();
    m_confidenceSlider->setOrientation(Qt::Horizontal);
    m_confidenceSlider->setMinimum(0);
    m_confidenceSlider->setMaximum(50);
    m_confidenceSlider->setTickInterval(1);
    m_confidenceSlider->setValue(int(10*m_confidenceLevel));
    m_confidenceSlider->setStyleSheet("QSlider::handle:horizontal {background-color: rgba(0, 0, 0, 0);}");

    QLabel *thresholdLabel = new QLabel(tr("Set Threshold"));
    m_thresholdSlider = new QSlider();
    m_thresholdSlider->setOrientation(Qt::Horizontal);
    m_thresholdSlider->setMinimum(0);
    m_thresholdSlider->setMaximum(50);
    m_thresholdSlider->setTickInterval(1);
    m_thresholdSlider->setValue(int(10*m_settings->getConfidenceThreshold()));
    connect(m_thresholdSlider, &QSlider::valueChanged, this, &MainWindow::setConfidenceLevel);

    QVBoxLayout *tempoLayout = new QVBoxLayout;
    tempoLayout->addLayout(tempoLine);
    tempoLayout->addWidget(m_lockCheckBox);
    //tempoLayout->addStretch(1);
    //tempoLayout->addWidget(m_filterCheckBox);
    tempoLayout->addWidget(m_wrongTempoButton);
    tempoLayout->addWidget(confidenceLabel);
    tempoLayout->addWidget(m_confidenceSlider);
    tempoLayout->addWidget(thresholdLabel);
    tempoLayout->addWidget(m_thresholdSlider);
    m_tempoGroup->setLayout(tempoLayout);


    // Layout for adjusting tempo
    m_limitGroup = new QGroupBox(tr("Adjust Tempo"));

    m_limitCheckBox = new QCheckBox(tr("Enable limits"));
    m_limitCheckBox->setChecked(m_limitTempo);
    connect(m_limitCheckBox, &QCheckBox::clicked, this, &MainWindow::updateTempo);

    QLabel *minLabel = new QLabel(tr("Min"));
    QLabel *maxLabel = new QLabel(tr("Max"));
    m_lowerBpmLine = new QLineEdit();
    m_upperBpmLine = new QLineEdit();
    m_lowerBpmLine->setMaxLength(5);
    m_upperBpmLine->setMaxLength(5);
    m_lowerBpmLine->setMaximumWidth(50);
    m_upperBpmLine->setMaximumWidth(50);
    QLabel *bpm1 = new QLabel(tr("bpm"));
    QLabel *bpm2 = new QLabel(tr("bpm"));
    updateLowerTempoLimit(m_settings->getTempoLowerLimit());
    updateUpperTempoLimit(m_settings->getTempoUpperLimit());
    connect(m_lowerBpmLine, &QLineEdit::editingFinished, this, &MainWindow::readLowerLimit);
    connect(m_upperBpmLine, &QLineEdit::editingFinished, this, &MainWindow::readUpperLimit);
    QHBoxLayout *minLine = new QHBoxLayout;
    minLine->addWidget(minLabel);
    minLine->addWidget(m_lowerBpmLine);
    minLine->addWidget(bpm1);
    QHBoxLayout *maxLine = new QHBoxLayout;
    maxLine->addWidget(maxLabel);
    maxLine->addWidget(m_upperBpmLine);
    maxLine->addWidget(bpm2);

    m_tempoMultiplierLabel = new QLabel;
    m_tempoMultiplierSlider = new QSlider;
    m_tempoMultiplierSlider->setOrientation(Qt::Horizontal);
    m_tempoMultiplierSlider->setMinimum(-2);
    m_tempoMultiplierSlider->setMaximum(2);
    m_tempoMultiplierSlider->setTickInterval(1);
    m_tempoMultiplierSlider->setValue(log2(m_settings->getCurrentLoopTempoMultiplier()));
    m_tempoMultiplierLabel->setText("Tempo multiplier " + QString::number(m_settings->getCurrentLoopTempoMultiplier(), 'f', 2));
    connect(m_tempoMultiplierSlider, &QSlider::valueChanged, this, &MainWindow::setTempoMultiplier);

    QGridLayout *limitLayout = new QGridLayout;
    limitLayout->addWidget(m_limitCheckBox, 0, 0, 1, 3);
    limitLayout->addWidget(minLabel, 1, 0, 1, 1);
    limitLayout->addWidget(m_lowerBpmLine, 1, 1, 1, 1);
    limitLayout->addWidget(bpm1, 1, 2, 1, 1);
    limitLayout->addWidget(maxLabel, 2, 0, 1, 1);
    limitLayout->addWidget(m_upperBpmLine, 2, 1, 1, 1);
    limitLayout->addWidget(bpm2, 2, 2, 1, 1);
    limitLayout->addWidget(m_tempoMultiplierLabel, 4, 0, 1, 3);
    limitLayout->addWidget(m_tempoMultiplierSlider, 5, 0, 1, 3);
    m_limitGroup->setLayout(limitLayout);

    QHBoxLayout *tempoControls = new QHBoxLayout;
    tempoControls->addWidget(m_tempoGroup);
    tempoControls->addWidget(m_limitGroup);


    // Layout for audio device settings
    m_audioGroup = new QGroupBox(tr("Audio Device"));
    m_audioSelect = new QComboBox;
    QHBoxLayout *audioSelectLayout = new QHBoxLayout;
    audioSelectLayout->addWidget(m_audioSelect);
    m_audioGroup->setLayout(audioSelectLayout);


    // Layout for video settings
    m_videoGroup = new QGroupBox(tr("Video Options"));

    m_startFullScreenCheckBox = new QCheckBox(tr("Start in fullscreen mode"));
    m_startFullScreenCheckBox->setChecked(m_settings->getStartAsFullscreen());
    connect(m_startFullScreenCheckBox, &QCheckBox::clicked, this, &MainWindow::setStartFullScreen);

    m_tempoControlsCheckBox = new QCheckBox(tr("Show tempo controls in fullscreen"));
    m_tempoControlsCheckBox->setChecked(m_settings->getShowTempoControls());
    connect(m_tempoControlsCheckBox, &QCheckBox::clicked, this, &MainWindow::setShowTempoControls);

    QLabel *screenLabel = new QLabel(tr("Display:"));
    m_screenSelect = new QComboBox;
    m_screens = qApp->screens();
    for (auto it = m_screens.begin(); it != m_screens.end(); it++) {
        m_screenSelect->addItem(QString((*it)->name() + ": " + (*it)->manufacturer() + " " + (*it)->model()));
    }
    m_screenSelect->setCurrentIndex(m_settings->getScreenNumber());
    connect(m_screenSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::setScreenNumber);
    QHBoxLayout *screenLayout = new QHBoxLayout;
    screenLayout->addWidget(screenLabel, 1);
    screenLayout->addWidget(m_screenSelect, 2);

    m_reverseFramesCheckBox = new QCheckBox(tr("Add reversed frames to loop"));
    m_reverseFramesCheckBox->setChecked(m_settings->getCurrentLoopAddReversedFrames());
    connect(m_reverseFramesCheckBox, &QCheckBox::clicked, this, &MainWindow::setAddReversedFrames);

    m_loopSelect = new QComboBox;
    QDir directory("frames");
    QStringList videoLoops = directory.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (auto it = videoLoops.begin(); it != videoLoops.end(); it++) {
        m_loopSelect->addItem(*it);
    }
    if (videoLoops.contains(m_settings->getVideoLoopName())) {
        m_loopSelect->setCurrentText(m_settings->getVideoLoopName());
    } else {
        m_settings->setVideoLoopName(videoLoops[0]);
    }
    connect(m_loopSelect, &QComboBox::currentTextChanged, this, &MainWindow::setVideoLoop);

    QLabel *loopLabel = new QLabel(tr("Video:"));
    QHBoxLayout *loopLayout = new QHBoxLayout;
    loopLayout->addWidget(loopLabel, 1);
    loopLayout->addWidget(m_loopSelect, 2);

    m_videoLayout = new QGridLayout;
    m_videoLayout->addWidget(m_startFullScreenCheckBox, 0, 0, 1, 3);
    m_videoLayout->addWidget(m_tempoControlsCheckBox, 1, 0, 1, 3);
    m_videoLayout->addWidget(m_reverseFramesCheckBox, 2, 0, 1, 3);
    m_videoLayout->addWidget(loopLabel, 3, 0);
    m_videoLayout->addWidget(m_loopSelect, 3, 1, 1, 2);
    m_videoLayout->addWidget(screenLabel, 4, 0);
    m_videoLayout->addWidget(m_screenSelect, 4, 1, 1, 2);
    m_videoGroup->setLayout(m_videoLayout);


    // Top level Layout
    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_graphicsWidget, 5);
    m_layout->addLayout(tempoControls, 1);
    m_layout->addWidget(m_audioGroup, 1);
    m_layout->addWidget(m_videoGroup, 1);
    setLayout(m_layout);


    m_keySpacebar = new QShortcut(this);
    m_keySpacebar->setKey(Qt::Key_Space);
    connect(m_keySpacebar, &QShortcut::activated, this, &MainWindow::toggleManualTempo);




    /// Non-UI initializations

    // Rhythm data calculation
    m_rhythm = new RhythmExtractor(this);
    connect(m_rhythm, &RhythmExtractor::calculationReady, this, &MainWindow::receiveBPMCalculationResult);

    // Audio device in use
    bool showAllInputs = parser->isSet(QCommandLineOption("a"));
    m_audio = new AudioDevice(this, m_settings->getAudioDevice(), showAllInputs);
    connect(m_audio, &AudioDevice::dataReady, m_rhythm, &RhythmExtractor::calculateTempo);
    connect(m_audio, &AudioDevice::songDataReady, m_rhythm, &RhythmExtractor::calculateTempo);

    QStringList audioDevices = m_audio->getAudioDevices();
    for (auto it = audioDevices.begin(); it != audioDevices.end(); it++) {
        m_audioSelect->addItem(*it);
    }
    connect(m_audioSelect, &QComboBox::currentTextChanged, m_audio, &AudioDevice::changeAudioInput);
    connect(m_audio, &AudioDevice::invalidateData, this, &MainWindow::invalidateTrackData);

    // Get data from Spotify through D-Bus
    m_spotifyWatcher = new SpotifyWatcher;
    connect(m_spotifyWatcher, &SpotifyWatcher::trackChanged, this, &MainWindow::updateTrackInfo);
    connect(m_spotifyWatcher, &SpotifyWatcher::invalidateData, this, &MainWindow::invalidateTrackData);

    // Get audio data after we get signal from Spotify that is has changed track
    connect(this, &MainWindow::trackCalculationNeeded, m_audio, &AudioDevice::emitAndClearSongBuffer);

    // Save track tempo data to a database
    bool saveTrackData = !parser->isSet(QCommandLineOption("n"));
    m_trackDBManager = new DBManager(saveTrackData);

    // Timer running when song (and videoloop tempo) changes
    m_smoothTempoUpdateTimer = new QTimer(this);
    m_smoothTempoUpdateTimer->setInterval(1000);
    connect(m_smoothTempoUpdateTimer, &QTimer::timeout, this, &MainWindow::smoothUpdateTempo);

    // Get data from PulseAudio server and invalidate if there are unwanted programs also playing audio
    m_pulseaudioWatcher = new PulseaudioWatcher(m_settings->getPulseAudioAppName(), m_settings->getPulseAudioIgnoreList());
    m_pulseaudioWatcher->startPolling(1000);
    connect(m_pulseaudioWatcher, &PulseaudioWatcher::invalidateData, this, &MainWindow::invalidateTrackData);

    this->show();
    if (m_settings->getStartAsFullscreen()) {
        setVideoFullScreen();
    }
}

MainWindow::~MainWindow()
{
    delete m_pulseaudioWatcher;
    delete m_trackDBManager;
    delete m_spotifyWatcher;

    delete m_settings;

    delete m_graphicsWidget;
}

void MainWindow::updateTempo()
{
    setTempoLimited();
    float tempo;

    double multiplier = m_settings->getCurrentLoopTempoMultiplier();

    if (m_limitCheckBox->isChecked()) {
        tempo = m_tempoLimited;
    } else {
        tempo = m_tempo*multiplier;
    }
    m_graphicsWidget->setBpm(tempo);
    m_setBpmLine->setText(QString::number(tempo, 'f', 1));
}

void MainWindow::smoothUpdateTempo()
{
    if (abs(m_tempo - m_targetTempo) < abs(m_step)) {
        m_tempo = m_targetTempo;
        m_smoothTempoUpdateTimer->stop();
    }
    else {
        m_tempo = m_tempo - m_step;
    }
    updateTempo();
}

void MainWindow::setTempoLimited()
{
    double multiplier = m_settings->getCurrentLoopTempoMultiplier();
    float tempo = m_tempo*multiplier;
    double lower = m_settings->getTempoLowerLimit();
    double upper = m_settings->getTempoUpperLimit();
    while (tempo < lower) {
        tempo = tempo * 2.0;
    }
    while (tempo > upper) {
        tempo = tempo / 2.0;
    }

    m_tempoLimited = tempo;
}

bool detectHalf(float oldTempo, float newTempo)
{
    float diff = abs(oldTempo - newTempo*2.0);
    if (diff < 5.5) {
        return true;
    }
    return false;
}

bool detectDouble(float oldTempo, float newTempo)
{
    float diff = abs(oldTempo - newTempo/2.0);
    //qDebug("diff: %.2f", diff);
    if (diff < 3.5) {
        return true;
    }
    return false;
}

bool MainWindow::detectDoubleTempoJump(float newTempo)
{
    // Use fuzzy compare to detect possible double and half tempos
    bool halfFound = detectHalf(m_tempo, newTempo);
    bool doubleFound = detectDouble(m_tempo, newTempo);
    return halfFound || doubleFound;
}

void MainWindow::autoUpdateTempo(const TempoData& tempoData)
{
    /*TODO: Add filtering. Essentia sometimes outputs single values which vary significantly from
    others, making playback speed jumpy. To fix this, for example require two subsequent values to
    be close to each other.
    
    TODO: Remove half or double tempos found*/

    if (m_disableAutoTempo) {
        return;
    }
    
    m_confidenceLevel = tempoData.confidence;
    m_confidenceSlider->setValue(int(10*m_confidenceLevel));

    if (m_confidenceLevel < m_settings->getConfidenceThreshold()) {
        //qDebug("Confidence too low: %.2f, threshold %.2f", m_confidenceLevel, m_thresholdLevel);
        return;
    }

    float tempo = tempoData.BPM;
    // filter to integer, since most music uses metronomes with integer bpm
    tempo = round(tempo);
    if (m_filterDouble && detectDoubleTempoJump(tempo)) {
        qDebug("Double or half tempo detected.");
        return;
    }

    m_bpmBuffer.pop_front();
    m_bpmBuffer.push_back(tempo);

    float average = 0;
    for (auto it = m_bpmBuffer.begin(); it != m_bpmBuffer.end(); it++) {
        average += *it;
    }
    average = average/m_bpmBuffer.size();
    //qDebug("Newest tempo: %.1f | Confidence: %.2f | Average of last 5: %.1f", tempo, m_confidenceLevel, average);

    if (m_lockCheckBox->isChecked()) {
        return;
    }

    m_tempo = average;

    updateTempo();
}

void MainWindow::manualUpdateTempo()
{
    QString bpmText = m_setBpmLine->text();

    bool success;
    float tempo = bpmText.toFloat(&success);

    if (success) {
        if (tempo > 1.0) {
            m_tempo = tempo;
            updateTempo();
        }
    }
}

void MainWindow::updateLockCheckBox()
{
    if (m_lockCheckBox->isChecked()) {
        m_setBpmLinePalette = QPalette();
        m_setBpmLinePalette.setColor(QPalette::Text, Qt::black);
        m_setBpmLine->setPalette(m_setBpmLinePalette);
        m_setBpmLine->setReadOnly(false);
    } else {
        m_setBpmLinePalette = QPalette();
        m_setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
        m_setBpmLine->setPalette(m_setBpmLinePalette);
        m_setBpmLine->setReadOnly(true);
    }
}

void MainWindow::updateFilterCheckBox()
{
    m_filterDouble = m_lockCheckBox->isChecked();
}

void MainWindow::toggleManualTempo()
{
    m_lockCheckBox->setChecked(!m_lockCheckBox->isChecked());
    updateLockCheckBox();
}

void MainWindow::readLowerLimit()
{
    QString boxText = m_lowerBpmLine->text();

    bool success;
    float limit = boxText.toFloat(&success);
    
    if (success) {
        updateLowerTempoLimit(limit);
    }
}

void MainWindow::updateLowerTempoLimit(float limit)
{
    double upper = m_settings->getTempoUpperLimit();
    if (limit > 0.0) {
        // Keep at least 1 octave  between lower and upper limits
        if (limit > upper/2.0) {
            limit = upper/2.0;
        }
        m_settings->setTempoLowerLimit(limit);
    }

    m_lowerBpmLine->setText(QString::number(m_settings->getTempoLowerLimit(), 'f', 1));
}

void MainWindow::readUpperLimit()
{
    QString boxText = m_upperBpmLine->text();

    bool success;
    float limit = boxText.toFloat(&success);

    if (success) {
        updateUpperTempoLimit(limit);
    }
}

void MainWindow::updateUpperTempoLimit(float limit)
{
    double lower = m_settings->getTempoLowerLimit();
    if (limit > 1.0 && limit < 300.0) {
        // Keep at least 1 octave between lower and upper limits
        if (limit < lower*2.0) {
            limit = lower*2.0;
        }
        m_settings->setTempoUpperLimit(limit);
    }
    m_upperBpmLine->setText(QString::number(m_settings->getTempoUpperLimit(), 'f', 1));
}

void MainWindow::setVideoFullScreen()
{
    if (m_graphicsWidget->windowState() == Qt::WindowFullScreen) {
        m_graphicsWidget->setWindowState(Qt::WindowNoState);
        m_graphicsWidget->setParent(this);
        m_layout->insertWidget(0, m_graphicsWidget, 5);
        if (!m_settings->getShowTempoControls()) {
            show();
        } else {
            m_layout->addWidget(m_audioGroup, 1);
            m_layout->removeWidget(m_loopSelect);
            m_videoLayout->addWidget(m_loopSelect, 3, 1, 1, 2);
            m_videoGroup->setLayout(m_videoLayout);
            m_layout->addWidget(m_videoGroup, 1);
        }
    } else {
        m_graphicsWidget->setParent(nullptr);
        m_screens = qApp->screens();
        if (m_settings->getScreenNumber() > m_screens.size()-1) {
            //m_screenNumber = 0;
            m_settings->setScreenNumber(0);
        }
        QRect screen = m_screens[m_settings->getScreenNumber()]->geometry();
        m_graphicsWidget->setGeometry(screen);
        m_graphicsWidget->showFullScreen();
        if (!m_settings->getShowTempoControls()) {
            hide();
        } else {
            m_layout->removeWidget(m_audioGroup);
            m_layout->removeWidget(m_videoGroup);
            m_layout->addWidget(m_loopSelect);
            m_layout->invalidate();
            QTimer::singleShot(50, this, &MainWindow::fixSize);
        }
    }
}

void MainWindow::setScreenNumber(int idx)
{
    m_settings->setScreenNumber(idx);
}

void MainWindow::setConfidenceLevel(int value)
{
    m_settings->setConfidenceThreshold(double(value)/10.0);
}

void MainWindow::setTempoMultiplier(int value)
{
    double multiplier = pow(2.0, double(value));
    m_settings->setCurrentLoopTempoMultiplier(multiplier);
    QString multiplierStr = QString::number(multiplier, 'f', 2);
    m_tempoMultiplierLabel->setText("Tempo multiplier " + multiplierStr);
    
    updateTempo();
}

void MainWindow::setStartFullScreen()
{
    m_settings->setStartAsFullscreen(m_startFullScreenCheckBox->isChecked());
}

void MainWindow::setShowTempoControls()
{
    m_settings->setShowTempoControls(m_tempoControlsCheckBox->isChecked());
}

void MainWindow::setAddReversedFrames()
{
    bool addReversedFrames = m_reverseFramesCheckBox->isChecked();

    m_settings->setCurrentLoopAddReversedFrames(addReversedFrames);
    m_graphicsWidget->setAddReversedFrames(addReversedFrames);
}

void MainWindow::setVideoLoop(QString loopName)
{
    m_settings->setVideoLoopName(loopName);
    m_graphicsWidget->setFrameFolder(loopName);

    double multiplier = m_settings->getCurrentLoopTempoMultiplier();
    m_tempoMultiplierSlider->setValue(log2(multiplier));
    m_tempoMultiplierLabel->setText("Tempo multiplier " + QString::number(multiplier, 'f', 2));

    bool addReversedFrames = m_settings->getCurrentLoopAddReversedFrames();
    m_reverseFramesCheckBox->setChecked(addReversedFrames);
    setAddReversedFrames();

    updateTempo();
}

void MainWindow::fixSize()
{
    resize(sizeHint());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    m_settings->setAudioDevice(m_audio->getCurrentDevice());
    m_settings->setLimitTempo(m_limitCheckBox->isChecked());
    m_settings->setScreenNumber(m_screenSelect->currentIndex());
    m_settings->writeSettings("settings.JSON");
}

int MainWindow::checkInit()
{
    return m_initError;
}

int MainWindow::checkDirectories()
{
    if (!QDir("frames").exists()) {
        qInfo("\nERROR: frames folder not found.\n");
        m_initError = 1;
    } else if (QDir("frames").isEmpty()) {
        qInfo("\nERROR: frames folder is empty.\n");
        m_initError = 1;
    } else {
        // check that none of the folders is empty
        QDir directory("frames");
        QStringList videoLoops = directory.entryList(QDir::NoDotAndDotDot | QDir::Dirs);

        for (auto it = videoLoops.begin(); it != videoLoops.end(); it++) {
            QString path = "frames/" + *it + "/";
            QDir frameDir = QDir(path);
            QStringList frames = frameDir.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG", QDir::Files);
            if (frames.isEmpty()) {
                m_initError = 1;
                qInfo("\nERROR: Empty frame folder: %s", qPrintable(path));
                qInfo("Please add frames or remove the folder\n");
            } else {
                qDebug("Found %d frames in folder %s", frames.size(), qPrintable(path));
            }
        }
    }

    if (m_initError) {
        qInfo("Please run the program in a directory which contains the loop frames.\n"
            "The directory structure should be:\n\t"
            "./videoloop-by-tempo (the executable)\n\t"
            "./frames/video1/frame1.jpg\n\t"
            "./frames/video1/frame2.jpg\n\t"
            "./frames/video1/frame....png\n\t"
            "./frames/video2/frame1.png\n\t"
            "./frames/video2/frame....png\n"
            
            "Current working directory is:\n\t%s\n",
            qPrintable(QDir(".").absolutePath())
        );
        return 1;
    }
    return 0;
}

void MainWindow::saveTrackData()
{
    if (m_trackData.confidence < 0.5) {
        invalidateTrackData("Too low tempo analysis confidence");
    }
    
    if (!m_invalidTrackData && QString("").compare(m_trackData.trackId)) {
        m_trackDBManager->writeBPM(m_trackData);
    }
    else {
        m_trackDataInvalidationReasons.removeDuplicates();
        qDebug("Track data not saved. Reasons during last track:");
        for (auto item : m_trackDataInvalidationReasons) {
            qDebug() << " " << qPrintable(item);
        }
        m_trackDataInvalidationReasons.clear();
    }
    // Reset track data invalidation for the next track
    m_invalidTrackData = false;
}

void MainWindow::updateTrackInfo(QString oldTrackId, QString oldArtist, QString oldTitle, QString newTrackId)
{
    m_trackData.trackId = oldTrackId;
    m_trackData.artist = oldArtist;
    m_trackData.title = oldTitle;

    m_currentTrackId = newTrackId;

    // if tempo data exists for the track which started playing
    float newBpm = m_trackDBManager->getBPM(newTrackId);
    if (newBpm > 0.0) {
        m_disableAutoTempo = true;

        m_targetTempo = newBpm;
        m_step = (m_tempo - m_targetTempo)/7.0; // TODO: Make non-hardcoded?
        m_smoothTempoUpdateTimer->start();

    }
    else {
        m_disableAutoTempo = false;
    }

    emit trackCalculationNeeded();
}

void MainWindow::updateTrackTempo(const TempoData& data)
{
    m_trackData = MyTypes::TrackData(data,
                                     m_trackData.trackId,
                                     m_trackData.artist,
                                     m_trackData.title);

    saveTrackData();
}

void MainWindow::receiveBPMCalculationResult(const TempoData& data, MyTypes::AudioBufferType type)
{
    if (type == MyTypes::track) {
        updateTrackTempo(data);
    }
    else {
        autoUpdateTempo(data);
    }
}

void MainWindow::invalidateTrackData(QString reason)
{
    m_invalidTrackData = true;
    m_trackDataInvalidationReasons.append(reason); 
}

void MainWindow::removeCurrentTrack()
{
    m_trackDBManager->deleteTrack(m_currentTrackId);
}