#include "gui.h"
#include <QPushButton>
#include <QApplication>
#include <QPalette>
#include <QWindow>
#include <QScreen>

MainWindow::MainWindow()
{
    m_graphicsWidget = new OpenGLWidget();
    connect(m_graphicsWidget, &OpenGLWidget::toggleFullScreen, this, &MainWindow::setVideoFullScreen);
    connect(m_graphicsWidget, &OpenGLWidget::initReady, this, &MainWindow::updateTempo);
    connect(m_graphicsWidget, &OpenGLWidget::spacePressed, this, &MainWindow::toggleManualTempo);

    m_lockCheckBox = new QCheckBox(tr("Manual tempo"));
    connect(m_lockCheckBox, &QPushButton::clicked, this, &MainWindow::updateLockCheckbox);

    m_setBpmLine = new QLineEdit(QString::number(m_tempo, 'f', 1));
    m_setBpmLine->setMaxLength(5);
    m_setBpmLinePalette = QPalette();
    m_setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
    m_setBpmLine->setPalette(m_setBpmLinePalette);
    connect(m_setBpmLine, &QLineEdit::returnPressed, this, &MainWindow::manualUpdateTempo);

    m_limitCheckBox = new QCheckBox(tr("Limit tempo between:"));
    connect(m_limitCheckBox, &QCheckBox::clicked, this, &MainWindow::updateTempo);

    m_lowerBpmLine = new QLineEdit(); //QString::number(m_tempoLowerLimit, 'f', 1));
    connect(m_lowerBpmLine, &QLineEdit::editingFinished, this, &MainWindow::readLowerLimit);

    m_upperBpmLine = new QLineEdit(); //QString::number(m_tempoUpperLimit, 'f', 1));
    connect(m_upperBpmLine, &QLineEdit::editingFinished, this, &MainWindow::readUpperLimit);

    QPushButton *saveButton = new QPushButton(tr("Save settings"));
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveSettings);

    QHBoxLayout *tempoControlLayout = new QHBoxLayout;
    tempoControlLayout->addWidget(m_lockCheckBox, 3);
    tempoControlLayout->addWidget(m_setBpmLine, 1);
    tempoControlLayout->addWidget(m_limitCheckBox, 3);
    tempoControlLayout->addWidget(m_lowerBpmLine, 1);
    tempoControlLayout->addWidget(m_upperBpmLine, 1);
    tempoControlLayout->addWidget(saveButton, 3);

    QLabel *confidenceLabel = new QLabel(tr("Beat confidence level"));

    m_confidenceSlider = new QSlider();
    m_confidenceSlider->setOrientation(Qt::Horizontal);
    m_confidenceSlider->setMinimum(10);
    m_confidenceSlider->setMaximum(50);
    m_confidenceSlider->setTickInterval(1);
    connect(m_confidenceSlider, &QSlider::valueChanged, this, &MainWindow::setConfidenceLevel);

    QVBoxLayout *confidenceLayout = new QVBoxLayout;
    confidenceLayout->addWidget(confidenceLabel);
    confidenceLayout->addWidget(m_confidenceSlider);

    m_tempoMultiplierLabel = new QLabel(tr("Tempo multiplier 1.0"));
    m_tempoMultiplierSlider = new QSlider();
    m_tempoMultiplierSlider->setOrientation(Qt::Horizontal);
    m_tempoMultiplierSlider->setMinimum(-2);
    m_tempoMultiplierSlider->setMaximum(2);
    m_tempoMultiplierSlider->setTickInterval(1);
    connect(m_tempoMultiplierSlider, &QSlider::valueChanged, this, &MainWindow::setTempoMultiplier);

    QVBoxLayout *tempoMultiplierLayout = new QVBoxLayout;
    tempoMultiplierLayout->addWidget(m_tempoMultiplierLabel);
    tempoMultiplierLayout->addWidget(m_tempoMultiplierSlider);

    QHBoxLayout *slidersLayout = new QHBoxLayout;
    slidersLayout->addLayout(confidenceLayout);
    slidersLayout->addLayout(tempoMultiplierLayout);

    QLabel *screenLabel = new QLabel(tr("Display:"));

    m_screenSelect = new QComboBox;
    m_screens = qApp->screens();
    for (auto it = m_screens.begin(); it != m_screens.end(); it++) {
        m_screenSelect->addItem(QString((*it)->name() + ": " + (*it)->manufacturer() + " " + (*it)->model()));
    }
    connect(m_screenSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::setScreenNumber);

    QLabel *audioLabel = new QLabel(tr("Audio Device:"));

    m_audioSelect = new QComboBox;

    QHBoxLayout *audioSelectLayout = new QHBoxLayout;
    audioSelectLayout->addWidget(screenLabel);
    audioSelectLayout->addWidget(m_screenSelect);
    audioSelectLayout->addWidget(audioLabel);
    audioSelectLayout->addWidget(m_audioSelect);

    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_graphicsWidget, 5);
    m_layout->addLayout(tempoControlLayout, 1);
    m_layout->addLayout(slidersLayout, 1);
    m_layout->addLayout(audioSelectLayout, 1);
    setLayout(m_layout);

    readSettings();

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
    float tempoLimited = tempo;
    const float confidence = tempoData.second;

    if (confidence < m_confidenceLevel) {
        return;
    }

    m_bpmBuffer.pop_front();
    m_bpmBuffer.push_back(tempo);

    float average = 0;
    for (auto it = m_bpmBuffer.begin(); it != m_bpmBuffer.end(); it++) {
        average += *it;
    }
    average = average/m_bpmBuffer.size();
    qDebug("Newest tempo: %.1f | Average of last 5: %.1f", tempo, average);

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
    qDebug("Toggling manual tempo checkbox to %d", !m_lockCheckBox->isChecked());
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
    QFile loadFile(QStringLiteral("settings.JSON"));

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open settings.JSON");
        return;
    }

    QString settingsData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(settingsData.toUtf8()));

    if (!loadDoc.isObject()) {
        qWarning("Malformed settings.JSON file. Please fix or delete it.");
        return;
    }

    QJsonObject values = loadDoc.object();
    qDebug("settings.JSON read. Using settings:");


    // Audio device for sound monitoring
    QString device = "";
    if (!values.value("audio_device").isUndefined()) {
        device = values["audio_device"].toString();
    }
    qDebug("  audo_device: %s", qPrintable(device));
    m_device = device;


    // Enable tempo limiting
    bool limitTempo = false;
    if (!values.value("limit_tempo").isUndefined()) {
        limitTempo = values["limit_tempo"].toBool();
    }
    qDebug("  limit_tempo: %d", limitTempo);
    m_limitCheckBox->setChecked(limitTempo);


    // Tempo lower limit
    double lowerLimit = 60.0;
    if (!values.value("tempo_lower_limit").isUndefined()) {
        lowerLimit = values["tempo_lower_limit"].toDouble();
    }
    if (lowerLimit < 1.0) {
        lowerLimit = 1.0;
    }
    qDebug("  tempo_lower_limit: %f", lowerLimit);
    m_tempoLowerLimit = lowerLimit;

    // Tempo upper limit
    double upperLimit = 120.0;
    if (!values.value("tempo_upper_limit").isUndefined()) {
        upperLimit = values["tempo_upper_limit"].toDouble();
    }
    if (upperLimit < 0) {
        upperLimit = 1.0;
    }
    qDebug("  tempo_upper_limit: %f", upperLimit);
    m_tempoUpperLimit = upperLimit;

    updateLowerTempoLimit(lowerLimit);
    updateUpperTempoLimit(upperLimit);


    // Screen for showing the video
    int screen = 0;
    if (!values.value("screen").isUndefined()) {
        screen = values["screen"].toInt();
    }
    if (screen < 0) {
        screen = 0;
    }
    m_screenNumber = screen;
    qDebug("  screen: %d", screen);
    m_screenSelect->setCurrentIndex(screen);


    // Confifence level
    double confidence = 3.0;
    if (!values.value("confidence_threshold").isUndefined()) {
        confidence = values["confidence_threshold"].toDouble();
    }
    if (confidence < 1.0) {
        confidence = 1.0;
    } else if (confidence > 5.0) {
        confidence = 5.0;
    }
    qDebug("  confidence_threshold: %f", confidence);
    m_confidenceSlider->setValue(int(10*confidence));


    // Tempo multiplier
    float tempoMultiplier = 1.0;
    if (!values.value("tempo_multiplier").isUndefined()) {
        tempoMultiplier = values["tempo_multiplier"].toDouble();
    }
    // note: accurate comparisons work because these are powers of 2.
    if (tempoMultiplier < 0.25) {
        tempoMultiplier = 0.25;
    } else if (tempoMultiplier > 4.0) {
        tempoMultiplier = 4.0;
    }
    int exp = log2(tempoMultiplier);
    m_tempoMultiplier = tempoMultiplier;
    m_tempoMultiplierSlider->setValue(exp);
    qDebug("  tempo_multiplier: %f", tempoMultiplier);

}

void MainWindow::saveSettings()
{
    QFile saveFile("settings.JSON");

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Coudn't open settings file for saving. Is the folder write-protected?");
        return;
    }

    QJsonObject settingsData;
    settingsData["audio_device"] = m_audio->getCurrentDevice();
    settingsData["limit_tempo"] = m_limitCheckBox->isChecked();
    settingsData["tempo_lower_limit"] = m_tempoLowerLimit;
    settingsData["tempo_upper_limit"] = m_tempoUpperLimit;
    settingsData["screen"] = m_screenSelect->currentIndex();
    settingsData["confidence_threshold"] = m_confidenceLevel;
    settingsData["tempo_multiplier"] = m_tempoMultiplier;

    qDebug("Saving the following settings:");
    qDebug("  audio_device: %s", qPrintable(m_audio->getCurrentDevice()));
    qDebug("  limit_tempo: %d", m_limitCheckBox->isChecked());
    qDebug("  tempo_lower_limit: %f", m_tempoLowerLimit);
    qDebug("  tempo_upper_limit: %f", m_tempoUpperLimit);
    qDebug("  screen: %d", m_screenSelect->currentIndex());
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
        this->show();
    } else {
        m_graphicsWidget->setParent(nullptr, Qt::Dialog);
        m_screens = qApp->screens();
        if (m_screenNumber > m_screens.size()-1) {
            m_screenNumber = 0;
        }
        QRect screen = m_screens[m_screenNumber]->geometry();
        m_graphicsWidget->setGeometry(screen);
        m_graphicsWidget->showFullScreen();
        this->hide();
    }
}

void MainWindow::setScreenNumber(int idx)
{
    m_screenNumber = idx;
    qDebug("Set fullscreen video to appear on display number %d", m_screenNumber);
}

void MainWindow::setConfidenceLevel(int value)
{
    m_confidenceLevel = float(value)/10.0;
    qDebug("Confidence level set to %f (valid range 0..5.32)", m_confidenceLevel);
}

void MainWindow::setTempoMultiplier(int value)
{
    m_tempoMultiplier = pow(2.0, float(value));
    QString multiplierStr = QString::number(m_tempoMultiplier, 'f', 2);
    m_tempoMultiplierLabel->setText("Tempo multiplier " + multiplierStr);
    qDebug("Tempo multiplier set to %s", qPrintable(multiplierStr));
    
    updateTempo();
}