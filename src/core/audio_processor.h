//
// Created by Léo KRYS on 18/12/2024.
//

#ifndef AUDIO_EQUALIZER_AUDIO_PROCESSOR_H
#define AUDIO_EQUALIZER_AUDIO_PROCESSOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Equalizer parameter structure
    typedef struct {
        float gains[10];        // Gains for each slice (in dB)
        float frequencies[10];  // Central slice frequence
        float q_factors[10];    // Q factor of filtres
    } equalizer_params_t;

    // Audio buffer structure
    typedef struct {
        float* data;           // Audio data
        uint32_t size;         // Buffer size
        uint32_t sample_rate;  // sample rate
        uint16_t channels;     // Number of channel
    } audio_buffer_t;

    // Main functions
    void process_audio_block(audio_buffer_t* buffer, equalizer_params_t* params);
    void initialize_equalizer(equalizer_params_t* params);
    void apply_equalizer_band(float* data, uint32_t size, float sample_rate,
                             float frequency, float gain, float q_factor);

#ifdef __cplusplus
}
#endif

#endif