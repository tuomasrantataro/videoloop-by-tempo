#include "gui.h"
#include "vulkanwindow.h"
#include <QPushButton>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPalette>

MainWindow::MainWindow(VulkanWindow *vulkanWindow) : m_window(vulkanWindow)
{
    QWidget *wrapper = QWidget::createWindowContainer(vulkanWindow);

    m_rhythm = new RhythmExtractor();
    connect(m_rhythm, &RhythmExtractor::tempoReady, this, &MainWindow::autoUpdateTempo);

    m_audio = new AudioDevice(this, defaultDevice);
    connect(m_audio, &AudioDevice::dataReady, this, &MainWindow::calculateTempo);

    lockCheckBox = new QCheckBox(tr("Manual tempo"));
    connect(lockCheckBox, &QPushButton::clicked, this, &MainWindow::updateLockCheckbox);

    setBpmLine = new QLineEdit(QString::number(m_tempo, 'f', 1));
    setBpmLine->setMaxLength(5);
    setBpmLinePalette = QPalette();
    setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
    setBpmLine->setPalette(setBpmLinePalette);
    connect(setBpmLine, &QLineEdit::returnPressed, this, &MainWindow::manualUpdateTempo);

    limitCheckBox = new QCheckBox(tr("Limit tempo between:"));
    connect(limitCheckBox, &QCheckBox::clicked, this, &MainWindow::updateTempo);

    lowerBpmLine = new QLineEdit(QString::number(m_tempoLowerLimit, 'f', 1));
    connect(lowerBpmLine, &QLineEdit::editingFinished, this, &MainWindow::updateLowerTempoLimit);

    upperBpmLine = new QLineEdit(QString::number(m_tempoUpperLimit, 'f', 1));
    connect(upperBpmLine, &QLineEdit::editingFinished, this, &MainWindow::updateUpperTempoLimit);

    QPushButton *saveButton = new QPushButton(tr("Save settings"));
    //connect

    QHBoxLayout *tempoControlLayout = new QHBoxLayout;
    tempoControlLayout->addWidget(lockCheckBox, 3);
    tempoControlLayout->addWidget(setBpmLine, 1);
    tempoControlLayout->addWidget(limitCheckBox, 3);
    tempoControlLayout->addWidget(lowerBpmLine, 1);
    tempoControlLayout->addWidget(upperBpmLine, 1);
    tempoControlLayout->addWidget(saveButton, 3);

    QLabel *audioLabel = new QLabel(tr("Audio Device:"));

    audioSelect = new QComboBox;

    QHBoxLayout *audioSelectLayout = new QHBoxLayout;
    audioSelectLayout->addWidget(audioLabel);
    audioSelectLayout->addWidget(audioSelect);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(wrapper, 5);
    layout->addLayout(tempoControlLayout, 1);
    layout->addLayout(audioSelectLayout, 1);
    setLayout(layout);
}

void MainWindow::updateTempo()
{
    float tempo;

    if (limitCheckBox->isChecked()) {
        tempo = m_tempoLimited;
    } else {
        tempo = m_tempo;
    }

    m_window->setBpm(tempo);
    setBpmLine->setText(QString::number(tempo, 'f', 1));
}

void MainWindow::setTempoLimited()
{
    float tempo = m_tempo;
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
    be close to each other.*/

    float tempo = tempoData.first;
    float tempoLimited = tempo;
    const float confidence = tempoData.second;

    if (lockCheckBox->isChecked()) {
        return;
    }
    if (confidence < 3.0) {
        return;
    }

    m_tempo = tempo;
    setTempoLimited();

    updateTempo();
}

void MainWindow::manualUpdateTempo()
{
    QString bpmText = setBpmLine->text();

    bool success;
    float tempo = bpmText.toFloat(&success);

    if (success) {
        if (tempo > 1.0) {
            m_tempo = tempo;
            setTempoLimited();
            updateTempo();
        }
    }
}

void MainWindow::updateLockCheckbox()
{
    if (lockCheckBox->isChecked()) {
        setBpmLinePalette = QPalette();
        setBpmLinePalette.setColor(QPalette::Text, Qt::black);
        setBpmLine->setPalette(setBpmLinePalette);
        setBpmLine->setReadOnly(false);
    } else {
        setBpmLinePalette = QPalette();
        setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
        setBpmLine->setPalette(setBpmLinePalette);
        setBpmLine->setReadOnly(true);
    }
}

void MainWindow::calculateTempo(std::vector<uint8_t> data)
{
    m_rhythm->calculateTempo(data);
}

void MainWindow::updateLowerTempoLimit()
{
    QString boxText = lowerBpmLine->text();

    bool success;
    float limit = boxText.toFloat(&success);

    if (success) {
        if (limit > 0.0) {
            // Keep at least 1 octave  between lower and upper limits
            if (limit > m_tempoUpperLimit/2.0) {
                limit = m_tempoUpperLimit/2.0;
            }
            m_tempoLowerLimit = limit;
        }
    }
    lowerBpmLine->setText(QString::number(m_tempoLowerLimit, 'f', 1));
}

void MainWindow::updateUpperTempoLimit()
{
    QString boxText = upperBpmLine->text();

    bool success;
    float limit = boxText.toFloat(&success);

    if (success) {
        if (limit > 1.0 && limit < 300.0) {
            // Keep at least 1 octave between lower and upper limits
            if (limit < m_tempoLowerLimit*2.0) {
                limit = m_tempoLowerLimit*2.0;
            }
            m_tempoUpperLimit = limit;
        }
    }
    upperBpmLine->setText(QString::number(m_tempoUpperLimit, 'f', 1));
}