#include "mainwindow.h"
#include <QApplication>
#include <QScreen>

using namespace MyTypes;

MainWindow::MainWindow(QCommandLineParser *parser) : m_parser(parser)
{
    qRegisterMetaType<TempoData>("TempoData");
    //NOTE: why this is needed in addition to AudioData
    qRegisterMetaType<std::vector<uint8_t>>("std::vector<uint8_t>");
    qRegisterMetaType<AudioData>("AudioData");
    qRegisterMetaType<AudioBufferType>("AudioBufferType");
    
    m_settings = new Settings();
    m_settings->readSettings("settings.JSON");

    m_graphics = new GraphicsWidget(m_settings->getVideoLoopName());
    m_graphics->setParent(this);
    connect(m_graphics, &GraphicsWidget::toggleFullScreen, this, &MainWindow::setVideoFullScreen);

    m_tempoHandler = new Tempo(this);
    connect(m_tempoHandler, &Tempo::tempoChanged, this, &MainWindow::updateTempo);
    connect(m_tempoHandler, &Tempo::tempoChanged, [=](float newTempo) { m_currentTempo = newTempo; });
    connect(m_tempoHandler, &Tempo::tempoLowerLimitChanged, this, &MainWindow::setLowerTempoLimit);
    connect(m_tempoHandler, &Tempo::tempoUpperLimitChanged, this, &MainWindow::setUpperTempoLimit);

    // Rhythm data calculation
    m_rhythm = new RhythmExtractor(this);
    connect(m_rhythm, &RhythmExtractor::calculationReady, this, &MainWindow::receiveBPMCalculationResult);

    // Audio device in use
    bool showAllInputs = parser->isSet(QCommandLineOption("a"));
    m_audio = new AudioDevice(this, m_settings->getAudioDevice(), showAllInputs);
    connect(m_audio, &AudioDevice::dataReady, m_rhythm, &RhythmExtractor::calculateTempo);
    connect(m_audio, &AudioDevice::songDataReady, m_rhythm, &RhythmExtractor::calculateTempo);

    // Handle whole track audiodata
    m_trackHandler = new Track(this,
                               !parser->isSet(QCommandLineOption("n")),
                               m_settings->getPulseAudioAppName(),
                               m_settings->getPulseAudioIgnoreList());
    connect(m_trackHandler, &Track::trackCalculationNeeded, m_audio, &AudioDevice::emitAndClearSongBuffer);
    connect(m_trackHandler, &Track::disableAutoTempo, m_tempoHandler, &Tempo::disableAutomaticTempo);
    connect(m_trackHandler, &Track::trackTempoFound, m_tempoHandler, &Tempo::setTempoSmooth);

    initUI();

    this->show();
    if (m_settings->getStartAsFullscreen()) {
        setVideoFullScreen();
    }
}

MainWindow::~MainWindow()
{
    delete m_settings;

    //delete m_graphicsWidget;
    m_graphics->deleteLater();
}

void MainWindow::initUI()
{
    m_tempoGroup = new QGroupBox(tr("Tempo"));

    m_lockCheckBox = new QCheckBox(tr("Manual tempo"), this);
    connect(m_lockCheckBox, &QPushButton::clicked, this, &MainWindow::updateLockCheckBox);
    connect(m_lockCheckBox, &QPushButton::toggled, m_tempoHandler, &Tempo::setEnableManualTempo);

    m_setBpmLine = new QLineEdit(QString::number(m_tempoHandler->getTempo(), 'f', 1));
    m_setBpmLine->setMaxLength(5);
    m_setBpmLine->setMaximumWidth(50);
    m_setBpmLinePalette = QPalette();
    m_setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
    m_setBpmLine->setPalette(m_setBpmLinePalette);
    connect(m_setBpmLine, &QLineEdit::returnPressed, this, &MainWindow::bpmLineChanged);

    QLabel *bpmLabel = new QLabel(tr("bpm"));
    QHBoxLayout *tempoLine = new QHBoxLayout;
    tempoLine->addWidget(m_setBpmLine);
    tempoLine->addWidget(bpmLabel);


    QPushButton *m_wrongTempoButton = new QPushButton(tr("Wrong tempo"));
    connect(m_wrongTempoButton, &QPushButton::clicked, m_trackHandler, &Track::removeCurrentTrack);


    QLabel *confidenceLabel = new QLabel(tr("BPM confidence"));
    m_confidenceSlider = new QSlider();
    m_confidenceSlider->setOrientation(Qt::Horizontal);
    m_confidenceSlider->setMinimum(0);
    m_confidenceSlider->setMaximum(50);
    m_confidenceSlider->setTickInterval(1);
    m_confidenceSlider->setValue(int(10*m_tempoHandler->getConfidenceThreshold()));
    m_confidenceSlider->setStyleSheet("QSlider::handle:horizontal {background-color: rgba(0, 0, 0, 0);}");

    QLabel *thresholdLabel = new QLabel(tr("Set Threshold"));
    m_thresholdSlider = new QSlider();
    m_thresholdSlider->setOrientation(Qt::Horizontal);
    m_thresholdSlider->setMinimum(0);
    m_thresholdSlider->setMaximum(50);
    m_thresholdSlider->setTickInterval(1);
    m_thresholdSlider->setValue(int(10*m_settings->getConfidenceThreshold()));
    connect(m_thresholdSlider, &QSlider::valueChanged, this, &MainWindow::setConfidenceThreshold);


    QVBoxLayout *tempoLayout = new QVBoxLayout;
    tempoLayout->addLayout(tempoLine);
    tempoLayout->addWidget(m_lockCheckBox);
    //tempoLayout->addStretch(1);
    tempoLayout->addWidget(m_wrongTempoButton);
    tempoLayout->addWidget(confidenceLabel);
    tempoLayout->addWidget(m_confidenceSlider);
    tempoLayout->addWidget(thresholdLabel);
    tempoLayout->addWidget(m_thresholdSlider);
    m_tempoGroup->setLayout(tempoLayout);


    // Layout for adjusting tempo
    m_limitGroup = new QGroupBox(tr("Adjust Tempo"));

    m_limitCheckBox = new QCheckBox(tr("Enable limits"));
    m_limitCheckBox->setChecked(m_tempoHandler->getEnableTempoLimits());
    connect(m_limitCheckBox, &QCheckBox::toggled, m_tempoHandler, &Tempo::setEnableTempoLimits);

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
    m_tempoHandler->setTempoLowerLimit(m_settings->getTempoLowerLimit());
    m_tempoHandler->setTempoUpperLimit(m_settings->getTempoUpperLimit());
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

    QStringList audioDevices = m_audio->getAudioDevices();
    for (auto it = audioDevices.begin(); it != audioDevices.end(); it++) {
        m_audioSelect->addItem(*it);
    }
    connect(m_audioSelect, &QComboBox::currentTextChanged, m_audio, &AudioDevice::changeAudioInput);
    connect(m_audio, &AudioDevice::invalidateData, m_trackHandler, &Track::invalidateTrackData);


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
    connect(m_reverseFramesCheckBox, &QCheckBox::toggled, this, &MainWindow::setAddReversedFrames);
    m_reverseFramesCheckBox->setChecked(m_settings->getCurrentLoopAddReversedFrames());

    m_loopSelect = new QComboBox;

    QDir directory("./assets/frames");
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
    //connect(m_loopSelect, &QComboBox::textHighlighted, m_graphics, &GraphicsWidget::loadFrames);
    connect(m_loopSelect, &QComboBox::textHighlighted, m_graphics, &GraphicsWidget::loadFutureFrames);

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
    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_graphics, 5);
    m_layout->addLayout(tempoControls, 1);
    m_layout->addWidget(m_audioGroup, 1);
    m_layout->addWidget(m_videoGroup, 1);
    setLayout(m_layout);


    m_keySpacebar = new QShortcut(this);
    m_keySpacebar->setKey(Qt::Key_Space);
    connect(m_keySpacebar, &QShortcut::activated, this, &MainWindow::toggleManualTempo);
}

void MainWindow::updateTempo(double tempo)
{
    double multiplier = m_settings->getCurrentLoopTempoMultiplier();

    m_graphics->setTempo(tempo*multiplier);
    m_setBpmLine->setText(QString::number(tempo, 'f', 1));
}

void MainWindow::bpmLineChanged()
{
    QString bpmText = m_setBpmLine->text();

    bool success;
    float tempo = bpmText.toFloat(&success);

    if (success) {
        if (tempo > 1.0) {
            m_tempoHandler->setTempoManual(tempo);
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
        m_tempoHandler->setTempoLowerLimit(limit);
    }
}

void MainWindow::setLowerTempoLimit(double limit)
{
    m_settings->setTempoLowerLimit(limit);
    m_lowerBpmLine->setText(QString::number(m_settings->getTempoLowerLimit(), 'f', 1));
}

void MainWindow::readUpperLimit()
{
    QString boxText = m_upperBpmLine->text();

    bool success;
    float limit = boxText.toFloat(&success);

    if (success) {
        m_tempoHandler->setTempoUpperLimit(limit);
    }
}

void MainWindow::setUpperTempoLimit(double limit)
{
    m_settings->setTempoUpperLimit(limit);
    m_upperBpmLine->setText(QString::number(m_settings->getTempoUpperLimit(), 'f', 1));
}

void MainWindow::setVideoFullScreen()
{
    if (m_graphics->windowState() == Qt::WindowFullScreen) {
        m_graphics->setWindowState(Qt::WindowNoState);
        m_graphics->setParent(this);
        m_layout->insertWidget(0, m_graphics, 5);
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
        m_graphics->setParent(nullptr);
        m_screens = qApp->screens();
        if (m_settings->getScreenNumber() > m_screens.size()-1) {
            m_settings->setScreenNumber(0);
        }
        QRect screen = m_screens[m_settings->getScreenNumber()]->geometry();
        m_graphics->setGeometry(screen);
        m_graphics->showFullScreen();
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

void MainWindow::setTempoMultiplier(int value)
{
    double multiplier = pow(2.0, double(value));
    m_settings->setCurrentLoopTempoMultiplier(multiplier);
    QString multiplierStr = QString::number(multiplier, 'f', 2);
    m_tempoMultiplierLabel->setText("Tempo multiplier " + multiplierStr);
    updateTempo(m_currentTempo);
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
    m_graphics->addReversedFrames(addReversedFrames);
}

void MainWindow::setVideoLoop(QString loopName)
{
    m_settings->setVideoLoopName(loopName);
    m_graphics->setVideoLoop(loopName);

    double multiplier = m_settings->getCurrentLoopTempoMultiplier();
    m_tempoMultiplierSlider->setValue(log2(multiplier));
    m_tempoMultiplierLabel->setText("Tempo multiplier " + QString::number(multiplier, 'f', 2));

    bool addReversedFrames = m_settings->getCurrentLoopAddReversedFrames();
    m_reverseFramesCheckBox->setChecked(addReversedFrames);
    setAddReversedFrames();
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

void MainWindow::receiveBPMCalculationResult(const TempoData& data, MyTypes::AudioBufferType type)
{
    if (type == MyTypes::track) {
        m_trackHandler->updateTrackTempo(data);
    }
    else {
        m_confidenceSlider->setValue(int(10*data.confidence));
        m_tempoHandler->setTempoAutomatic(data);
        
    }
}

void MainWindow::setConfidenceThreshold(int value)
{
    double value_ = (double)value/10.0;
    m_settings->setConfidenceThreshold(value_);
    m_tempoHandler->setConfidenceThreshold(value_);
}