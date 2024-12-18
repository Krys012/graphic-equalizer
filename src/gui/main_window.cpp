//
// Created by Léo KRYS on 18/12/2024.
//
#include "main_window.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , audioBackend(new AudioBackend(this))
{
    setupUi();
    setupConnections();
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Création des contrôles
    equalizer = new EqualizerWidget(this);
    spectrum = new SpectrumDisplay(this);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    openButton = new QPushButton("Ouvrir", this);
    playButton = new QPushButton("Play", this);
    stopButton = new QPushButton("Stop", this);
    timeLabel = new QLabel("00:00", this);

    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(playButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(timeLabel);
    buttonLayout->addStretch();

    // Ajout au layout principal
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(spectrum);
    mainLayout->addWidget(equalizer);

    setCentralWidget(centralWidget);
    setWindowTitle("Audio Equalizer");
    resize(800, 600);

    // Configuration du timer pour la mise à jour du spectre
    updateTimer = new QTimer(this);
    updateTimer->setInterval(50); // 20 fps
}

void MainWindow::setupConnections() {
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(playButton, &QPushButton::clicked, this, &MainWindow::playPause);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(audioBackend, &AudioBackend::positionChanged,
            this, &MainWindow::updateTime);
    connect(audioBackend, &AudioBackend::spectrumChanged,
            this, &MainWindow::updateSpectrumDisplay);
}

void MainWindow::initializeAudio() {
    initialize_equalizer(&equalizerParams);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Ouvrir un fichier audio"), "",
        tr("Fichiers WAV (*.wav);;Tous les fichiers (*)"));

    if (fileName.isEmpty())
        return;

    if (!audioBackend->loadFile(fileName)) {
        QMessageBox::critical(this, "Erreur",
            "Impossible de charger le fichier audio.");
        return;
    }
}

void MainWindow::playPause() {
    if (!audioBackend) return;

    if (!isPlaying) {
        audioBackend->play();
        playButton->setText("Pause");
    } else {
        audioBackend->pause();
        playButton->setText("Play");
    }
    isPlaying = !isPlaying;

    qDebug() << "État de lecture changé :" << (isPlaying ? "Playing" : "Paused");
}

void MainWindow::stop() {
    audioBackend->stop();
    isPlaying = false;
    playButton->setText("Play");
    timeLabel->setText("00:00");
}

void MainWindow::updateTime(qint64 position) {
    if (!audioBackend || !audioBackend->getRawAudioBuffer())
        return;

    int sampleRate = audioBackend->getRawAudioBuffer()->sample_rate;
    int seconds = position / sampleRate;
    int minutes = seconds / 60;
    seconds %= 60;

    timeLabel->setText(QString("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0')));
}

void MainWindow::updateSpectrumDisplay() {
    if (audioBackend) {
        const std::vector<float>& spectrumData = audioBackend->getSpectrum();
        spectrum->updateSpectrum(spectrumData);
    }
}

void MainWindow::equalizerChanged(int band, int value) {
    Q_UNUSED(band); // Pour éviter l'avertissement de paramètre non utilisé
    std::vector<float> gains(10);
    for (int i = 0; i < 10; i++) {
        gains[i] = value;
    }
    audioBackend->setEqualizerGains(gains);
}
