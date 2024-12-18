#ifndef AUDIO_BACKEND_H
#define AUDIO_BACKEND_H

#include <QObject>
#include <QTimer>
#include <QIODevice>
#include <QAudioSink>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QByteArray>
#include "../core/audio_processor.h"
#include "../core/fft.h"
#include <vector>

// Classe pour la gestion du buffer audio
class AudioBuffer : public QIODevice {
    Q_OBJECT
public:
    explicit AudioBuffer(QObject *parent = nullptr);
    void setAudioData(audio_buffer_t* data);
    bool atEnd() const override;
    qint64 pos() const override { return position; }
    bool reset() override { position = 0; return true; }

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    audio_buffer_t* audioData;
    qint64 position;
    qint64 bufferSize;
    static const int CHUNK_SIZE = 2048;
    void logBufferState() const;
};

class AudioBackend : public QObject {
    Q_OBJECT
public:
    explicit AudioBackend(QObject *parent = nullptr);
    ~AudioBackend();

    bool loadFile(const QString& filename);
    void play();
    void pause();
    void stop();
    void setEqualizerGains(const std::vector<float>& gains);
    const audio_buffer_t* getRawAudioBuffer() const { return rawAudioBuffer; }
    const std::vector<float>& getSpectrum() const { return spectrumData; }
    signals:
        void positionChanged(qint64 position);
    void spectrumChanged();
    void durationChanged(qint64 duration);

private slots:
    void handleStateChanged(QAudio::State state);
    void processBuffer();
    void updateSpectrum();

private:
    QAudioDevice findAudioDevice();
    QAudioFormat getAudioFormat();
    void logAudioFormat(const QAudioFormat& format);
    void initializeAudio();
    void computeSpectrum();

    QAudioSink* audioOutput;
    AudioBuffer* audioBuffer;
    audio_buffer_t* rawAudioBuffer;
    equalizer_params_t equalizerParams;
    QTimer* spectrumTimer;
    std::vector<float> spectrumData;

    // Buffer pour la FFT
    cplx* fftBuffer;
    static const int FFT_SIZE = 2048;
};

#endif // AUDIO_BACKEND_H