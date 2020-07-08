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

    float old_bpm = 126.34;
    setBpmLine = new QLineEdit(QString::number(old_bpm, 'f', 1));
    setBpmLine->setMaxLength(5);
    setBpmLinePalette = QPalette();
    setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
    setBpmLine->setPalette(setBpmLinePalette);
    connect(setBpmLine, &QLineEdit::returnPressed, this, &MainWindow::manualUpdateTempo);

    limitCheckBox = new QCheckBox(tr("Limit tempo between:"));
    //connect

    float lower = 60.0;
    QLineEdit *lowerBpmEdit = new QLineEdit(QString::number(lower, 'f', 1));
    //connect

    float upper = 120.0;
    QLineEdit *upperBpmEdit = new QLineEdit(QString::number(upper, 'f', 1));
    //connect

    QPushButton *saveButton = new QPushButton(tr("Save settings"));
    //connect

    QHBoxLayout *tempoControlLayout = new QHBoxLayout;
    tempoControlLayout->addWidget(lockCheckBox, 3);
    tempoControlLayout->addWidget(setBpmLine, 1);
    tempoControlLayout->addWidget(limitCheckBox, 3);
    tempoControlLayout->addWidget(lowerBpmEdit, 1);
    tempoControlLayout->addWidget(upperBpmEdit, 1);
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

void MainWindow::updateTempo(float bpm)
{
    m_window->setBpm(bpm);
    setBpmLine->setText(QString::number(m_tempo, 'f', 1));
}

void MainWindow::autoUpdateTempo(tempoPair tempoData)
{
    /*TODO: Add filtering. Essentia sometimes outputs single values which vary significantly from
    others, making playback speed jumpy. To fix this, for example require two subsequent values to
    be close to each other.*/

    float tempo = tempoData.first;
    m_tempo = tempo;
    const float confidence = tempoData.second;

    if (lockCheckBox->isChecked()) {
        return;
    }
    if (confidence < 3.0) {
        return;
    }

    if (limitCheckBox->isChecked()) {
        while (tempo < tempoLowerLimit) {
            tempo = tempo * 2.0;
        }
        while (tempo > tempoUpperLimit) {
            tempo = tempo / 2.0;
        }
    }

    updateTempo(tempo);
}

void MainWindow::manualUpdateTempo()
{
    QString bpmText = setBpmLine->text();

    bool success;
    float bpm = bpmText.toFloat(&success);

    if (success) {
        if (bpm > 1.0) {
            updateTempo(bpm);
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