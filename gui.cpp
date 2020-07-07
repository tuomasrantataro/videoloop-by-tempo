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
    connect(m_rhythm, &RhythmExtractor::tempoReady, this, &MainWindow::autoUpdateBpm);

    m_audio = new AudioDevice(this, defaultDevice);
    connect(m_audio, &AudioDevice::dataReady, this, &MainWindow::calculateBPM);

    lockCheckBox = new QCheckBox(tr("Manual tempo"));
    connect(lockCheckBox, &QPushButton::clicked, this, &MainWindow::updateLockCheckbox);

    float old_bpm = 126.34;
    setBpmLine = new QLineEdit(QString::number(old_bpm, 'f', 1));
    setBpmLine->setMaxLength(5);
    setBpmLinePalette = QPalette();
    setBpmLinePalette.setColor(QPalette::Text, Qt::gray);
    setBpmLine->setPalette(setBpmLinePalette);
    connect(setBpmLine, &QLineEdit::returnPressed, this, &MainWindow::updateBpmManually);

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

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(wrapper, 5);
    layout->addLayout(tempoControlLayout, 1);
    setLayout(layout);
}

void MainWindow::changePlayBackRate(float bpm)
{
    if (int(bpm) != int(oldBpm)) {
        //qWarning("bpm to change: %f", bpm);
        oldBpm = bpm;
        m_window->setBpm(bpm);
    }
}

void MainWindow::autoUpdateBpm(tempoPair tempo)
{
    if (tempo.second > 3.0) {
        updateBpm(tempo.first, false);
    }
}

void MainWindow::updateBpm(float bpm, bool manual)
{
    if (!manual) {
        if (lockCheckBox->isChecked()) {
            return;
        }
        bpm = float(int(bpm + 0.5));
    }
    if (limitCheckBox->isChecked()) {
        while (bpm < tempoLowerLimit) {
            bpm = bpm * 2.0;
        }
        while (bpm > tempoUpperLimit) {
            bpm = bpm / 2.0;
        }
    }

    changePlayBackRate(bpm);
    setBpmLine->setText(QString::number(oldBpm, 'f', 1));

}

void MainWindow::updateBpmManually()
{
    QString bpmText = setBpmLine->text();

    bool success;
    float bpm = bpmText.toFloat(&success);

    if (success) {
        if (bpm > 1.0) {
            updateBpm(bpm, true);
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

void MainWindow::calculateBPM(std::vector<uint8_t> data)
{
    m_rhythm->calculateTempo(data);
}