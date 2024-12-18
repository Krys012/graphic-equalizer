//
// Created by Léo KRYS on 18/12/2024.
//

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTimer>
#include <vector>
#include "equalizer_widget.h"
#include "spectrum_display.h"
#include "../core/audio_processor.h"
#include "./audio_backend.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    private slots:
        void openFile();
    void playPause();
    void stop();
    void updateSpectrumDisplay();
    void equalizerChanged(int band, int value);

private:
    void setupUi();
    void setupConnections();
    void initializeAudio();

    EqualizerWidget* equalizer;
    SpectrumDisplay* spectrum;
    QPushButton* openButton;
    QPushButton* playButton;
    QPushButton* stopButton;
    QLabel* timeLabel;
    QTimer* updateTimer;

    audio_buffer_t* audioBuffer;
    equalizer_params_t equalizerParams;
    bool isPlaying;

    AudioBackend* audioBackend;
    void updateTime(qint64 position);
};

#endif // MAIN_WINDOW_H