//
// Created by Léo KRYS on 18/12/2024.
//

#include "audio_backend.h"
#include <QDebug>

#include "wav_reader.h"

AudioBuffer::AudioBuffer(QObject *parent)
    : QIODevice(parent)
    , audioData(nullptr)
    , position(0)
{
    open(QIODevice::ReadOnly);
}

void AudioBuffer::setAudioData(audio_buffer_t* data) {
    audioData = data;
    position = 0;
}

bool AudioBuffer::atEnd() const {
    if (!audioData) return true;
    qint64 totalSize = static_cast<qint64>(audioData->size) * sizeof(float);
    return position >= totalSize;
}

qint64 AudioBuffer::readData(char *data, qint64 maxlen) {
    if (!audioData || atEnd())
        return 0;

    qint64 samples = maxlen / sizeof(float);
    qint64 remaining = (audioData->size * sizeof(float)) - position;
    qint64 bytesToRead = qMin(maxlen, remaining);
    samples = bytesToRead / sizeof(float);

    // Convertir les float en samples PCM 16-bit
    int16_t* outputBuffer = reinterpret_cast<int16_t*>(data);
    float* inputBuffer = audioData->data + (position / sizeof(float));

    for (qint64 i = 0; i < samples; i++) {
        outputBuffer[i] = static_cast<int16_t>(inputBuffer[i] * 32767.0f);
    }

    position += bytesToRead;
    return bytesToRead;
}

qint64 AudioBuffer::writeData(const char *data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0; // Lecture seule
}

AudioBackend::AudioBackend(QObject *parent)
    : QObject(parent)
    , audioOutput(nullptr)
    , audioBuffer(new AudioBuffer(this))
    , rawAudioBuffer(nullptr)
    , spectrumTimer(new QTimer(this))
    , fftBuffer(nullptr)
{
    initialize_equalizer(&equalizerParams);
    fftBuffer = new cplx[FFT_SIZE];
    spectrumData.resize(FFT_SIZE/2);

    spectrumTimer->setInterval(50); // 20 fps
    connect(spectrumTimer, &QTimer::timeout, this, &AudioBackend::updateSpectrum);
}

AudioBackend::~AudioBackend() {
    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
    }
    if (rawAudioBuffer) {
        free_audio_buffer(rawAudioBuffer);
    }
    delete[] fftBuffer;
}

void AudioBackend::initializeAudio() {
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    const auto &defaultDeviceInfo = QMediaDevices::defaultAudioOutput();
    if (!defaultDeviceInfo.isFormatSupported(format)) {
        qWarning() << "Format audio non supporté";
        format = defaultDeviceInfo.preferredFormat();
    }

    if (audioOutput) {
        delete audioOutput;
    }
    audioOutput = new QAudioSink(format, this);
}

bool AudioBackend::loadFile(const QString& filename) {
    if (rawAudioBuffer) {
        free_audio_buffer(rawAudioBuffer);
    }

    rawAudioBuffer = read_wav_file(filename.toLocal8Bit().constData());
    if (!rawAudioBuffer) {
        return false;
    }

    audioBuffer->setAudioData(rawAudioBuffer);
    initializeAudio();
    emit durationChanged(rawAudioBuffer->size);
    return true;
}

void AudioBackend::play() {
    if (!audioOutput || !rawAudioBuffer)
        return;

    if (audioOutput->state() != QAudio::State::ActiveState) {
        audioOutput->start(audioBuffer);
        spectrumTimer->start();
    }
}

void AudioBackend::pause() {
    if (audioOutput && audioOutput->state() == QAudio::State::ActiveState) {
        audioOutput->suspend();
        spectrumTimer->stop();
    }
}

void AudioBackend::stop() {
    if (audioOutput) {
        audioOutput->stop();
        audioBuffer->reset();
        spectrumTimer->stop();
    }
}

void AudioBackend::setEqualizerGains(const std::vector<float>& gains) {
    if (gains.size() != 10)
        return;

    for (int i = 0; i < 10; i++) {
        equalizerParams.gains[i] = gains[i];
    }

    if (rawAudioBuffer) {
        process_audio_block(rawAudioBuffer, &equalizerParams);
    }
}

void AudioBackend::processBuffer() {
    qint64 position = audioBuffer->pos() / sizeof(float);
    emit positionChanged(position);
}

void AudioBackend::updateSpectrum() {
    computeSpectrum();
    emit spectrumChanged();
}

void AudioBackend::computeSpectrum() {
    if (!rawAudioBuffer)
        return;

    // Position actuelle dans le buffer
    qint64 currentPos = audioBuffer->pos() / sizeof(float);

    // Copier les données pour la FFT
    for (int i = 0; i < FFT_SIZE; i++) {
        if (currentPos + i < rawAudioBuffer->size) {
            fftBuffer[i] = rawAudioBuffer->data[currentPos + i];
        } else {
            fftBuffer[i] = 0;
        }
    }

    // Appliquer la fenêtre de Hanning
    for (int i = 0; i < FFT_SIZE; i++) {
        double window = 0.5 * (1 - cos(2 * M_PI * i / (FFT_SIZE - 1)));
        fftBuffer[i] *= window;
    }

    // Calculer la FFT
    fft(fftBuffer, FFT_SIZE);

    // Calculer les magnitudes
    for (int i = 0; i < FFT_SIZE/2; i++) {
        spectrumData[i] = std::abs(fftBuffer[i]) / FFT_SIZE;
    }
}
