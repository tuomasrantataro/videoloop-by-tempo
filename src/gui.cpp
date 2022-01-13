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
    
    readSettings();

    m_graphicsWidget = new OpenGLWidget(m_loopName);
    connect(m_graphicsWidget, &OpenGLWidget::toggleFullScreen, this, &MainWindow::setVideoFullScreen);
    connect(m_graphicsWidget, &OpenGLWidget::initReady, this, &MainWindow::updateTempo);
    connect(m_graphicsWidget, &OpenGLWidget::initReady, this, &MainWindow::setAddReversedFrames);
    connect(m_graphicsWidget, &OpenGLWidget::spacePressed, this, &MainWindow::toggleManualTempo);


    // Layout for setting tempo
    m_tempoGroup = new QGroupBox(tr("Tempo"));

    m_lockCheckBox = new QCheckBox(tr("Manual tempo"));
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

    m_filterCheckBox = new QCheckBox(tr("Filter half/double tempo"));
    m_filterCheckBox->setChecked(m_filterDouble);
    connect(m_filterCheckBox, &QPushButton::clicked, this, &MainWindow::updateFilterCheckBox);

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
    m_thresholdSlider->setValue(int(10*m_thresholdLevel));
    connect(m_thresholdSlider, &QSlider::valueChanged, this, &MainWindow::setConfidenceLevel);

    QVBoxLayout *tempoLayout = new QVBoxLayout;
    tempoLayout->addLayout(tempoLine);
    tempoLayout->addWidget(m_lockCheckBox);
    //tempoLayout->addStretch(1);
    tempoLayout->addWidget(m_filterCheckBox);
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
    updateLowerTempoLimit(m_tempoLowerLimit);
    updateUpperTempoLimit(m_tempoUpperLimit);
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
    m_tempoMultiplierSlider->setValue(log2(m_tempoMultiplier));
    m_tempoMultiplierLabel->setText("Tempo multiplier " + QString::number(m_tempoMultiplier, 'f', 2));
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
    m_startFullScreenCheckBox->setChecked(m_startAsFullScreen);
    connect(m_startFullScreenCheckBox, &QCheckBox::clicked, this, &MainWindow::setStartFullScreen);

    m_tempoControlsCheckBox = new QCheckBox(tr("Show tempo controls in fullscreen"));
    m_tempoControlsCheckBox->setChecked(m_showTempoControls);
    connect(m_tempoControlsCheckBox, &QCheckBox::clicked, this, &MainWindow::setShowTempoControls);

    QLabel *screenLabel = new QLabel(tr("Display:"));
    m_screenSelect = new QComboBox;
    m_screens = qApp->screens();
    for (auto it = m_screens.begin(); it != m_screens.end(); it++) {
        m_screenSelect->addItem(QString((*it)->name() + ": " + (*it)->manufacturer() + " " + (*it)->model()));
    }
    m_screenSelect->setCurrentIndex(m_screenNumber);
    connect(m_screenSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::setScreenNumber);
    QHBoxLayout *screenLayout = new QHBoxLayout;
    screenLayout->addWidget(screenLabel, 1);
    screenLayout->addWidget(m_screenSelect, 2);

    m_reverseFramesCheckBox = new QCheckBox(tr("Add reversed frames to loop"));
    m_reverseFramesCheckBox->setChecked(m_addReversedFrames);
    connect(m_reverseFramesCheckBox, &QCheckBox::clicked, this, &MainWindow::setAddReversedFrames);

    m_loopSelect = new QComboBox;
    QDir directory("frames");
    QStringList videoLoops = directory.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (auto it = videoLoops.begin(); it != videoLoops.end(); it++) {
        m_loopSelect->addItem(*it);
    }
    if (videoLoops.contains(m_loopName)) {
        m_loopSelect->setCurrentText(m_loopName);
    } else {
        m_loopName = videoLoops[0];
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
    m_audio = new AudioDevice(this, m_device, showAllInputs);
    connect(m_audio, &AudioDevice::dataReady, m_rhythm, &RhythmExtractor::calculateTempo);
    connect(m_audio, &AudioDevice::songDataReady, m_rhythm, &RhythmExtractor::calculateTempo);

    QStringList audioDevices = m_audio->getAudioDevices();
    for (auto it = audioDevices.begin(); it != audioDevices.end(); it++) {
        m_audioSelect->addItem(*it);
    }
    connect(m_audioSelect, &QComboBox::currentTextChanged, m_audio, &AudioDevice::changeAudioInput);
    connect(m_audio, &AudioDevice::deviceChanged, this, &MainWindow::invalidateTrackData);

    // Get data from Spotify through D-Bus
    m_dbusWatcher = new DBusWatcher;
    connect(m_dbusWatcher, &DBusWatcher::trackChanged, this, &MainWindow::updateTrackInfo);  // track changed
    connect(m_dbusWatcher, &DBusWatcher::invalidateData, this, &MainWindow::invalidateTrackData);

    // Get data after we get signal from Spotify that is has changed track
    connect(this, &MainWindow::trackCalculationNeeded, m_audio, &AudioDevice::emitAndClearSongBuffer);

    // Save track tempo data to a database
    bool saveTrackData = !parser->isSet(QCommandLineOption("n"));
    m_trackDBManager = new DBManager(saveTrackData);



    this->show();
    if (m_startAsFullScreen) {
        setVideoFullScreen();
    }
}

void MainWindow::updateTempo()
{
    setTempoLimited();
    float tempo;

    if (m_limitCheckBox->isChecked()) {
        tempo = m_tempoLimited;
    } else {
        tempo = m_tempo*m_tempoMultiplier;
    }
    m_graphicsWidget->setBpm(tempo);
    m_setBpmLine->setText(QString::number(tempo, 'f', 1));
}

void MainWindow::setTempoLimited()
{
    float tempo = m_tempo*m_tempoMultiplier;
    while (tempo < m_tempoLowerLimit) {
        tempo = tempo * 2.0;
    }
    while (tempo > m_tempoUpperLimit) {
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
    qDebug("diff: %.2f", diff);
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

    if (m_confidenceLevel < m_thresholdLevel) {
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
    if (limit > 0.0) {
        // Keep at least 1 octave  between lower and upper limits
        if (limit > m_tempoUpperLimit/2.0) {
            limit = m_tempoUpperLimit/2.0;
        }
        m_tempoLowerLimit = limit;
    }

    m_lowerBpmLine->setText(QString::number(m_tempoLowerLimit, 'f', 1));
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
    if (limit > 1.0 && limit < 300.0) {
        // Keep at least 1 octave between lower and upper limits
        if (limit < m_tempoLowerLimit*2.0) {
            limit = m_tempoLowerLimit*2.0;
        }
        m_tempoUpperLimit = limit;
    }
    m_upperBpmLine->setText(QString::number(m_tempoUpperLimit, 'f', 1));
}

void MainWindow::readSettings()
{
    QJsonObject values;
    QJsonDocument loadDoc;

    QFile loadFile(QStringLiteral("settings.JSON"));

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open settings.JSON. Using default settings.");
    } else {
        QString settingsData = loadFile.readAll();
        loadDoc = QJsonDocument::fromJson(settingsData.toUtf8());
    }

    if (loadFile.exists() && !loadDoc.isObject()) {
        qWarning("Malformed settings.JSON file. Using default settings.");
    } else {
        values = loadDoc.object();
    }

    // Audio device for sound monitoring
    m_device = "";
    if (!values.value("audio_device").isUndefined()) {
        m_device = values["audio_device"].toString();
    }


    // Enable tempo limiting
    m_limitTempo = false;
    if (!values.value("limit_tempo").isUndefined()) {
        m_limitTempo = values["limit_tempo"].toBool();
    }

    // Enable double/half tempo filtering
    m_filterDouble = false;
    if (!values.value("filter_double").isUndefined()) {
        m_filterDouble = values["filter_double"].toBool();
    }


    // Tempo lower limit
    m_tempoLowerLimit = 60.0;
    if (!values.value("tempo_lower_limit").isUndefined()) {
        m_tempoLowerLimit = values["tempo_lower_limit"].toDouble();
    }
    if (m_tempoLowerLimit < 1.0) {
        m_tempoLowerLimit = 1.0;
    }

    // Tempo upper limit
    m_tempoUpperLimit = 120.0;
    if (!values.value("tempo_upper_limit").isUndefined()) {
        m_tempoUpperLimit = values["tempo_upper_limit"].toDouble();
    }
    if (m_tempoUpperLimit < 2*m_tempoLowerLimit) {
        m_tempoUpperLimit = 2*m_tempoLowerLimit;
    }


    // Screen for showing the video
    m_screenNumber = 0;
    if (!values.value("screen").isUndefined()) {
        m_screenNumber = values["screen"].toInt();
    }
    if (m_screenNumber < 0) {
        m_screenNumber = 0;
    }


    // Start as fullscreen
    m_startAsFullScreen = false;
    if (!values.value("start_as_fullscreen").isUndefined()) {
        m_startAsFullScreen = values["start_as_fullscreen"].toBool();
    }


    // Show controls when in fullscreen
    m_showTempoControls = false;
    if (!values.value("show_tempo_controls").isUndefined()) {
        m_showTempoControls = values["show_tempo_controls"].toBool();
    }


    // Toggle adding frames reversed to the video loop
    m_addReversedFrames = false;
    if (!values.value("add_reversed_frames").isUndefined()) {
        m_addReversedFrames = values["add_reversed_frames"].toBool();
    }


    // Default video
    m_loopName = "";
    if (!values.value("video_name").isUndefined()) {
        m_loopName = values["video_name"].toString();
    }


    // Confifence level
    m_thresholdLevel = 3.0;
    if (!values.value("confidence_threshold").isUndefined()) {
        m_thresholdLevel = values["confidence_threshold"].toDouble();
    }
    if (m_thresholdLevel < 1.0) {
        m_thresholdLevel = 1.0;
    } else if (m_thresholdLevel > 5.0) {
        m_thresholdLevel = 5.0;
    }


    // Tempo multiplier
    m_tempoMultiplier = 1.0;
    if (!values.value("tempo_multiplier").isUndefined()) {
        m_tempoMultiplier = values["tempo_multiplier"].toDouble();
    }
    // note: accurate comparisons work because these are powers of 2.
    if (m_tempoMultiplier < 0.25) {
        m_tempoMultiplier = 0.25;
    } else if (m_tempoMultiplier > 4.0) {
        m_tempoMultiplier = 4.0;
    }


    qDebug("Starting with settings:");
    qDebug("  audo_device: %s", qPrintable(m_device));
    qDebug("  limit_tempo: %d", m_limitTempo);
    qDebug("  filter_double: %d", m_filterDouble);
    qDebug("  tempo_lower_limit: %f", m_tempoLowerLimit);
    qDebug("  tempo_upper_limit: %f", m_tempoUpperLimit);
    qDebug("  screen: %d", m_screenNumber);
    qDebug("  start_as_fullscreen: %d", m_startAsFullScreen);
    qDebug("  show_tempo_controls: %d", m_showTempoControls);
    qDebug("  add_reversed_frames: %d", m_addReversedFrames);
    qDebug("  video_name: %s", qPrintable(m_loopName));
    qDebug("  confidence_threshold: %f", m_thresholdLevel);
    qDebug("  tempo_multiplier: %f", m_tempoMultiplier);
}

void MainWindow::saveSettings()
{
    QFile saveFile("settings.JSON");

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Coudn't save settings. Is the folder write-protected?");
        return;
    }

    QJsonObject settingsData;
    settingsData["audio_device"] = m_audio->getCurrentDevice();
    settingsData["limit_tempo"] = m_limitCheckBox->isChecked();
    settingsData["filter_double"] = m_filterCheckBox->isChecked();
    settingsData["tempo_lower_limit"] = m_tempoLowerLimit;
    settingsData["tempo_upper_limit"] = m_tempoUpperLimit;
    settingsData["screen"] = m_screenSelect->currentIndex();
    settingsData["start_as_fullscreen"] = m_startAsFullScreen;
    settingsData["show_tempo_controls"] = m_showTempoControls;
    settingsData["add_reversed_frames"] = m_addReversedFrames;
    settingsData["video_name"] = m_loopName;
    settingsData["confidence_threshold"] = m_thresholdLevel;
    settingsData["tempo_multiplier"] = m_tempoMultiplier;

    qDebug("Saving settings:");
    qDebug("  audio_device: %s", qPrintable(m_audio->getCurrentDevice()));
    qDebug("  limit_tempo: %d", m_limitCheckBox->isChecked());
    qDebug("  filter_double: %d", m_filterCheckBox->isChecked());
    qDebug("  tempo_lower_limit: %f", m_tempoLowerLimit);
    qDebug("  tempo_upper_limit: %f", m_tempoUpperLimit);
    qDebug("  screen: %d", m_screenSelect->currentIndex());
    qDebug("  start_as_fullscreen: %d", m_startAsFullScreen);
    qDebug("  show_tempo_controls: %d", m_showTempoControls);
    qDebug("  add_reversed_frames: %d", m_addReversedFrames);
    qDebug("  video_name: %s", qPrintable(m_loopName));
    qDebug("  confidence_threshold %f", m_thresholdLevel);
    qDebug("  tempo_multiplier: %f", m_tempoMultiplier);

    saveFile.write(QJsonDocument(settingsData).toJson());
}

void MainWindow::readLoopSettings(QString loopName)
{
    QJsonObject values;
    QJsonDocument loadDoc;

    QString settingsFileName = "frames/" + loopName + "/" + "settings.JSON";
    QFile loadFile(settingsFileName);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open loop settings.JSON. Using default settings.");
    } else {
        QString settingsData = loadFile.readAll();
        loadDoc = QJsonDocument::fromJson(settingsData.toUtf8());
    }

    if (loadFile.exists() && !loadDoc.isObject()) {
        qWarning("Malformed loop settings.JSON file. Using default settings.");
    } else {
        values = loadDoc.object();
    }

    // Toggle adding frames reversed to the video loop
    if (!values.value("add_reversed_frames").isUndefined()) {
        m_addReversedFrames = values["add_reversed_frames"].toBool();
    }
    m_reverseFramesCheckBox->setChecked(m_addReversedFrames);
    setAddReversedFrames();

    // Tempo multiplier
    if (!values.value("tempo_multiplier").isUndefined()) {
        m_tempoMultiplier = values["tempo_multiplier"].toDouble();
    }
    // note: accurate comparisons work because these are powers of 2.
    if (m_tempoMultiplier < 0.25) {
        m_tempoMultiplier = 0.25;
    } else if (m_tempoMultiplier > 4.0) {
        m_tempoMultiplier = 4.0;
    }
    m_tempoMultiplierSlider->setValue(log2(m_tempoMultiplier));
    m_tempoMultiplierLabel->setText("Tempo multiplier " + QString::number(m_tempoMultiplier, 'f', 2));

}

void MainWindow::writeLoopSettings(QString loopName)
{
    //Write video-specific settings to its frame folder
    QString settingsFileName = "frames/" + loopName + "/" + "settings.JSON";

    QFile saveFile(settingsFileName);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Coudn't save settings. Is the folder write-protected?");
        return;
    }

    QJsonObject settingsData;
    settingsData["add_reversed_frames"] = m_addReversedFrames;
    settingsData["tempo_multiplier"] = m_tempoMultiplier;

    saveFile.write(QJsonDocument(settingsData).toJson());

}

void MainWindow::setVideoFullScreen()
{
    if (m_graphicsWidget->windowState() == Qt::WindowFullScreen) {
        m_graphicsWidget->setWindowState(Qt::WindowNoState);
        m_graphicsWidget->setParent(this);
        m_layout->insertWidget(0, m_graphicsWidget, 5);
        if (!m_showTempoControls) {
            show();
        } else {
            m_layout->addWidget(m_audioGroup, 1);
            m_layout->removeWidget(m_loopSelect);
            //m_videoLayout->addWidget(m_reverseFramesCheckBox, 2, 0, 1, 3);
            m_videoLayout->addWidget(m_loopSelect, 3, 1, 1, 2);
            m_videoGroup->setLayout(m_videoLayout);
            m_layout->addWidget(m_videoGroup, 1);
        }
    } else {
        m_graphicsWidget->setParent(nullptr);
        m_screens = qApp->screens();
        if (m_screenNumber > m_screens.size()-1) {
            m_screenNumber = 0;
        }
        QRect screen = m_screens[m_screenNumber]->geometry();
        m_graphicsWidget->setGeometry(screen);
        m_graphicsWidget->showFullScreen();
        if (!m_showTempoControls) {
            hide();
        } else {
            m_layout->removeWidget(m_audioGroup);
            m_layout->removeWidget(m_videoGroup);
            //m_layout->addWidget(m_reverseFramesCheckBox);
            m_layout->addWidget(m_loopSelect);
            m_layout->invalidate();
            QTimer::singleShot(50, this, &MainWindow::fixSize);
        }
    }
}

void MainWindow::setScreenNumber(int idx)
{
    m_screenNumber = idx;
}

void MainWindow::setConfidenceLevel(int value)
{
    m_thresholdLevel = float(value)/10.0;
}

void MainWindow::setTempoMultiplier(int value)
{
    m_tempoMultiplier = pow(2.0, float(value));
    QString multiplierStr = QString::number(m_tempoMultiplier, 'f', 2);
    m_tempoMultiplierLabel->setText("Tempo multiplier " + multiplierStr);
    
    updateTempo();
}

void MainWindow::setStartFullScreen()
{
    m_startAsFullScreen = m_startFullScreenCheckBox->isChecked();
}

void MainWindow::setShowTempoControls()
{
    m_showTempoControls = m_tempoControlsCheckBox->isChecked();
}

void MainWindow::setAddReversedFrames()
{
    m_addReversedFrames = m_reverseFramesCheckBox->isChecked();
    m_graphicsWidget->setAddReversedFrames(m_addReversedFrames);
}

void MainWindow::setVideoLoop(QString loopName)
{
    writeLoopSettings(m_loopName);
    m_loopName = loopName;
    m_graphicsWidget->setFrameFolder(loopName);
    readLoopSettings(m_loopName);
    updateTempo();
}

void MainWindow::fixSize()
{
    resize(sizeHint());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    //TODO: instead of deleting, try reattaching the m_graphicsWidget so it gets deleted automatically
    delete m_graphicsWidget;
    saveSettings();
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
    if (!m_invalidTrackData && QString("").compare(m_trackData.trackId)) {
        m_trackDBManager->writeBPM(m_trackData);
        qDebug("Saving tempo data to track database:\n\t"
               "%s | %.1f bpm | confidence: %.2f\n\t%s - %s",
               qPrintable(m_trackData.trackId),
               m_trackData.BPM,
               m_trackData.confidence,
               qPrintable(m_trackData.artist),
               qPrintable(m_trackData.title));
    }
    else {
        qDebug("Data not saved to track database. Invalid: %d, trackid: %s",
                m_invalidTrackData, qPrintable(m_trackData.trackId));
    }
    // Reset track data invalidation for the next track
    m_invalidTrackData = false;
}

void MainWindow::updateTrackInfo(QString oldTrackId, QString oldArtist, QString oldTitle, QString newTrackId)
{
    m_trackData.trackId = oldTrackId;
    m_trackData.artist = oldArtist;
    m_trackData.title = oldTitle;

    // if tempo data exists for the track which started playing
    float newBpm = m_trackDBManager->getBPM(newTrackId);
    if (newBpm > 0.0) {
        m_disableAutoTempo = true;
        m_tempo = newBpm;

        updateTempo();
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

void MainWindow::invalidateTrackData()
{
    m_invalidTrackData = true;
}