//
// Created by Léo KRYS on 18/12/2024.
//
#include "audio_processor.h"
#include "fft.h"
#include <math.h>
#include <stdlib.h>

void initialize_equalizer(equalizer_params_t* params) {
    float freqs[10] = {32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};

    for (int i = 0; i < 10; i++) {
        params->frequencies[i] = freqs[i];
        params->gains[i] = 0.0f;  // gain neutre
        params->q_factors[i] = 1.4142f;  // Q standard pour un égaliseur
    }
}

void apply_equalizer_band(float* data, uint32_t size, float sample_rate,
                         float frequency, float gain, float q_factor) {
    int fft_size = size;
    cplx* fft_buffer = (cplx*)malloc(fft_size * sizeof(cplx));

    for (uint32_t i = 0; i < size; i++) {
        fft_buffer[i] = data[i];
    }

    fft(fft_buffer, fft_size);

    float omega = 2.0f * M_PI * frequency / sample_rate;
    float alpha = sin(omega) / (2.0f * q_factor);
    float A = pow(10.0f, gain / 40.0f);

    for (int i = 0; i < fft_size/2; i++) {
        float freq = i * sample_rate / fft_size;
        float response = 1.0f;

        if (freq > 0) {
            float freq_ratio = freq / frequency;
            response = A / sqrt(1 + pow(q_factor * (freq_ratio - 1.0f/freq_ratio), 2));
        }

        fft_buffer[i] *= response;
        if (i > 0 && i < fft_size/2) {
            fft_buffer[fft_size-i] *= response;
        }
    }

    ifft(fft_buffer, fft_size);

    for (uint32_t i = 0; i < size; i++) {
        data[i] = creal(fft_buffer[i]);
    }

    free(fft_buffer);
}

void process_audio_block(audio_buffer_t* buffer, equalizer_params_t* params) {
    for (int band = 0; band < 10; band++) {
        if (fabs(params->gains[band]) > 0.01f) {
            apply_equalizer_band(
                buffer->data,
                buffer->size,
                buffer->sample_rate,
                params->frequencies[band],
                params->gains[band],
                params->q_factors[band]
            );
        }
    }
}