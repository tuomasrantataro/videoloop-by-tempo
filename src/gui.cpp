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

MainWindow::MainWindow()
{
    readSettings();

    m_graphicsWidget = new OpenGLWidget(m_loopName);
    connect(m_graphicsWidget, &OpenGLWidget::toggleFullScreen, this, &MainWindow::setVideoFullScreen);
    connect(m_graphicsWidget, &OpenGLWidget::initReady, this, &MainWindow::updateTempo);
    connect(m_graphicsWidget, &OpenGLWidget::initReady, this, &MainWindow::setAddReversedFrames);
    connect(m_graphicsWidget, &OpenGLWidget::spacePressed, this, &MainWindow::toggleManualTempo);


    // Layout for setting tempo
    m_tempoGroup = new QGroupBox(tr("Tempo"));

    m_lockCheckBox = new QCheckBox(tr("Manual tempo"));
    connect(m_lockCheckBox, &QPushButton::clicked, this, &MainWindow::updateLockCheckbox);

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

    QLabel *confidenceLabel = new QLabel(tr("Detection threshold"));
    m_confidenceSlider = new QSlider();
    m_confidenceSlider->setOrientation(Qt::Horizontal);
    m_confidenceSlider->setMinimum(10);
    m_confidenceSlider->setMaximum(50);
    m_confidenceSlider->setTickInterval(1);
    m_confidenceSlider->setValue(int(10*m_confidenceLevel));
    connect(m_confidenceSlider, &QSlider::valueChanged, this, &MainWindow::setConfidenceLevel);

    QVBoxLayout *tempoLayout = new QVBoxLayout;
    tempoLayout->addLayout(tempoLine);
    tempoLayout->addWidget(m_lockCheckBox);
    tempoLayout->addStretch(1);
    tempoLayout->addWidget(confidenceLabel);
    tempoLayout->addWidget(m_confidenceSlider);
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
    limitLayout->addWidget(m_tempoMultiplierLabel, 3, 0, 1, 3);
    limitLayout->addWidget(m_tempoMultiplierSlider, 4, 0, 1, 3);
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

    QGridLayout *videoLayout = new QGridLayout;
    videoLayout->addWidget(m_startFullScreenCheckBox, 0, 0, 1, 3);
    videoLayout->addWidget(m_tempoControlsCheckBox, 1, 0, 1, 3);
    videoLayout->addWidget(m_reverseFramesCheckBox, 2, 0, 1, 3);
    videoLayout->addWidget(loopLabel, 3, 0);
    videoLayout->addWidget(m_loopSelect, 3, 1, 1, 2);
    videoLayout->addWidget(screenLabel, 4, 0);
    videoLayout->addWidget(m_screenSelect, 4, 1, 1, 2);
    m_videoGroup->setLayout(videoLayout);


    // Top level Layout
    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_graphicsWidget, 5);
    m_layout->addLayout(tempoControls, 1);
    m_layout->addWidget(m_audioGroup, 1);
    m_layout->addWidget(m_videoGroup, 1);
    setLayout(m_layout);


    // Non-layout initializations
    m_rhythm = new RhythmExtractor();
    connect(m_rhythm, &RhythmExtractor::tempoReady, this, &MainWindow::autoUpdateTempo);

    m_audio = new AudioDevice(this, m_device);
    connect(m_audio, &AudioDevice::dataReady, this, &MainWindow::calculateTempo);

    QStringList audioDevices = m_audio->getAudioDevices();
    for (auto it = audioDevices.begin(); it != audioDevices.end(); it++) {
        m_audioSelect->addItem(*it);
    }
    connect(m_audioSelect, &QComboBox::currentTextChanged, m_audio, &AudioDevice::changeAudioInput);

    m_keySpacebar = new QShortcut(this);
    m_keySpacebar->setKey(Qt::Key_Space);
    connect(m_keySpacebar, &QShortcut::activated, this, &MainWindow::toggleManualTempo);

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

void MainWindow::autoUpdateTempo(tempoPair tempoData)
{
    /*TODO: Add filtering. Essentia sometimes outputs single values which vary significantly from
    others, making playback speed jumpy. To fix this, for example require two subsequent values to
    be close to each other.
    
    TODO: Remove half or double tempos found*/

    float tempo = tempoData.first;
    //float tempoLimited = tempo;
    const float confidence = tempoData.second;

    if (confidence < m_confidenceLevel) {
        qDebug("Confidence too low: %.2f", confidence);
        return;
    }

    m_bpmBuffer.pop_front();
    m_bpmBuffer.push_back(tempo);

    float average = 0;
    for (auto it = m_bpmBuffer.begin(); it != m_bpmBuffer.end(); it++) {
        average += *it;
    }
    average = average/m_bpmBuffer.size();
    qDebug("Newest tempo: %.1f Confidence: %.2f | Average of last 5: %.1f", tempo, confidence, average);

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

void MainWindow::updateLockCheckbox()
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

void MainWindow::toggleManualTempo()
{
    m_lockCheckBox->setChecked(!m_lockCheckBox->isChecked());
    updateLockCheckbox();
}

void MainWindow::calculateTempo(std::vector<uint8_t> data)
{
    m_rhythm->calculateTempo(data);
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
    m_confidenceLevel = 3.0;
    if (!values.value("confidence_threshold").isUndefined()) {
        m_confidenceLevel = values["confidence_threshold"].toDouble();
    }
    if (m_confidenceLevel < 1.0) {
        m_confidenceLevel = 1.0;
    } else if (m_confidenceLevel > 5.0) {
        m_confidenceLevel = 5.0;
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
    qDebug("  tempo_lower_limit: %f", m_tempoLowerLimit);
    qDebug("  tempo_upper_limit: %f", m_tempoUpperLimit);
    qDebug("  screen: %d", m_screenNumber);
    qDebug("  start_as_fullscreen: %d", m_startAsFullScreen);
    qDebug("  show_tempo_controls: %d", m_showTempoControls);
    qDebug("  add_reversed_frames: %d", m_addReversedFrames);
    qDebug("  video_name: %s", qPrintable(m_loopName));
    qDebug("  confidence_threshold: %f", m_confidenceLevel);
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
    settingsData["tempo_lower_limit"] = m_tempoLowerLimit;
    settingsData["tempo_upper_limit"] = m_tempoUpperLimit;
    settingsData["screen"] = m_screenSelect->currentIndex();
    settingsData["start_as_fullscreen"] = m_startAsFullScreen;
    settingsData["show_tempo_controls"] = m_showTempoControls;
    settingsData["add_reversed_frames"] = m_addReversedFrames;
    settingsData["video_name"] = m_loopName;
    settingsData["confidence_threshold"] = m_confidenceLevel;
    settingsData["tempo_multiplier"] = m_tempoMultiplier;

    qDebug("Saving settings:");
    qDebug("  audio_device: %s", qPrintable(m_audio->getCurrentDevice()));
    qDebug("  limit_tempo: %d", m_limitCheckBox->isChecked());
    qDebug("  tempo_lower_limit: %f", m_tempoLowerLimit);
    qDebug("  tempo_upper_limit: %f", m_tempoUpperLimit);
    qDebug("  screen: %d", m_screenSelect->currentIndex());
    qDebug("  start_as_fullscreen: %d", m_startAsFullScreen);
    qDebug("  show_tempo_controls: %d", m_showTempoControls);
    qDebug("  add_reversed_frames: %d", m_addReversedFrames);
    qDebug("  video_name: %s", qPrintable(m_loopName));
    qDebug("  confidence_threshold %f", m_confidenceLevel);
    qDebug("  tempo_multiplier: %f", m_tempoMultiplier);

    saveFile.write(QJsonDocument(settingsData).toJson());
}

void MainWindow::setVideoFullScreen()
{
    if (m_graphicsWidget->windowState() == Qt::WindowFullScreen) {
        m_graphicsWidget->setParent(this, Qt::Widget);
        m_graphicsWidget->setWindowState(Qt::WindowNoState);
        m_layout->insertWidget(0, m_graphicsWidget, 5);
        if (!m_showTempoControls) {
            show();
        } else {
            m_layout->addWidget(m_audioGroup, 1);
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
    m_confidenceLevel = float(value)/10.0;
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
    m_loopName = loopName;
    m_graphicsWidget->setFrameFolder(loopName);
    updateTempo();
}

void MainWindow::fixSize()
{
    resize(sizeHint());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    delete m_graphicsWidget;
    saveSettings();
}