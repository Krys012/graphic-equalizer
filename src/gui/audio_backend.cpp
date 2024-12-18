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
    if (audioData) {
        qDebug() << "New audio data set:";
        qDebug() << " Sample rate:" << audioData->sample_rate;
        qDebug() << " Channels:" << audioData->channels;
        qDebug() << " Total samples:" << audioData->size;
    }
    logBufferState();
}

qint64 AudioBuffer::readData(char *data, qint64 maxSize) {
    if (!audioData || !data || maxSize <= 0) {
        return 0;
    }

    const int INPUT_RATE = 48000;
    const int OUTPUT_RATE = 44100;
    const qint64 bytesPerSample = sizeof(float);
    const int channels = audioData->channels;

    qint64 framesToRead = maxSize / (bytesPerSample * channels);
    float* outBuffer = reinterpret_cast<float*>(data);

    for (qint64 i = 0; i < framesToRead; ++i) {
        // Calculer la position exacte dans le buffer source
        double srcPos = i * double(INPUT_RATE) / OUTPUT_RATE;
        qint64 srcIdx = position / (bytesPerSample * channels) + qint64(srcPos);

        if (srcIdx >= audioData->size / channels) {
            return i * bytesPerSample * channels;
        }

        // Interpolation linéaire
        for (int ch = 0; ch < channels; ++ch) {
            outBuffer[i * channels + ch] = audioData->data[srcIdx * channels + ch];
        }
    }

    qint64 bytesRead = framesToRead * bytesPerSample * channels;
    position += bytesRead * INPUT_RATE / OUTPUT_RATE;

    return bytesRead;
}

bool AudioBuffer::atEnd() const {
    if (!audioData) return true;
    qint64 totalSize = static_cast<qint64>(audioData->size) * sizeof(float);
    return position >= totalSize;
}

void AudioBackend::initializeAudio() {
    if (!rawAudioBuffer)
        return;

    const auto &defaultDeviceInfo = QMediaDevices::defaultAudioOutput();
    QAudioFormat format = defaultDeviceInfo.preferredFormat();

    // Garder le format préféré du système (44100 Hz, Float)
    // mais adapter notre flux de données

    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
    }

    audioOutput = new QAudioSink(defaultDeviceInfo, format, this);

    int periodSize = (format.sampleRate() * format.channelCount() * format.bytesPerSample() * 100) / 1000;
    audioOutput->setBufferSize(periodSize);

    qDebug() << "Format final :";
    logAudioFormat(format);
    qDebug() << "Buffer size:" << audioOutput->bufferSize() << "bytes";

    connect(audioOutput, &QAudioSink::stateChanged,
            this, &AudioBackend::handleStateChanged);
}

qint64 AudioBuffer::writeData(const char *data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0; // Lecture seule
}

void AudioBuffer::logBufferState() const {
    qDebug() << "Buffer state:";
    qDebug() << " Position:" << position;
    qDebug() << " Total size:" << (audioData ? audioData->size * sizeof(float) : 0);
    qDebug() << " At end:" << atEnd();
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

bool AudioBackend::loadFile(const QString& filename) {
    qDebug() << "Loading file:" << filename;

    if (rawAudioBuffer) {
        free_audio_buffer(rawAudioBuffer);
        rawAudioBuffer = nullptr;
    }

    rawAudioBuffer = read_wav_file(filename.toLocal8Bit().constData());
    if (!rawAudioBuffer) {
        qDebug() << "Failed to load WAV file";
        return false;
    }

    qDebug() << "WAV file loaded:";
    qDebug() << " Sample rate:" << rawAudioBuffer->sample_rate;
    qDebug() << " Channels:" << rawAudioBuffer->channels;
    qDebug() << " Samples:" << rawAudioBuffer->size;

    audioBuffer->setAudioData(rawAudioBuffer);
    initializeAudio();

    emit durationChanged(rawAudioBuffer->size / rawAudioBuffer->sample_rate);
    return true;
}

void AudioBackend::handleStateChanged(QAudio::State state) {
    qDebug() << "Audio state changed:" << state;
    switch (state) {
        case QAudio::ActiveState:
            qDebug() << "Audio is active";
        break;
        case QAudio::SuspendedState:
            qDebug() << "Audio is suspended";
        break;
        case QAudio::StoppedState:
            qDebug() << "Audio is stopped";
        if (audioOutput->error() != QAudio::NoError) {
            qDebug() << "Error:" << audioOutput->error();
        }
        break;
        case QAudio::IdleState:
            qDebug() << "Audio is idle";
        break;
    }
}

void AudioBackend::play() {
    if (!audioOutput || !rawAudioBuffer)
        return;

    if (audioOutput->state() == QAudio::SuspendedState) {
        audioOutput->resume();
    } else {
        audioBuffer->reset();
        audioOutput->start(audioBuffer);
    }

    // Démarrer le timer pour le spectre
    spectrumTimer->start();
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
    if (!audioBuffer || !audioOutput)
        return;

    qint64 processedBytes = audioBuffer->pos();

    if (processedBytes > 0) {
        emit positionChanged(processedBytes / (sizeof(int16_t) * rawAudioBuffer->channels));
    }

    // Vérifier si nous sommes à la fin
    if (audioBuffer->atEnd()) {
        audioOutput->stop();
        audioBuffer->reset();
        spectrumTimer->stop();
        emit positionChanged(0);
    }
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

QAudioDevice AudioBackend::findAudioDevice() {
    const auto devices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &deviceInfo : devices) {
        qDebug() << "Found audio device:" << deviceInfo.description();
    }

    auto defaultDevice = QMediaDevices::defaultAudioOutput();
    if (defaultDevice.isNull()) {
        qWarning() << "Aucun périphérique audio trouvé";
        return QAudioDevice();
    }

    qDebug() << "Using device:" << defaultDevice.description();
    return defaultDevice;
}

QAudioFormat AudioBackend::getAudioFormat() {
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);  // Stéréo
    format.setSampleFormat(QAudioFormat::Int16);

    logAudioFormat(format);
    return format;
}

void AudioBackend::logAudioFormat(const QAudioFormat& format) {
    qDebug() << "Audio format:";
    qDebug() << " Sample rate:" << format.sampleRate();
    qDebug() << " Channels:" << format.channelCount();
    qDebug() << " Sample size:" << format.bytesPerSample();
    qDebug() << " Sample type:" << static_cast<int>(format.sampleFormat());
}
