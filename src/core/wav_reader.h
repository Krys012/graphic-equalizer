//
// Created by Léo KRYS on 18/12/2024.
//
#ifndef AUDIO_EQUALIZER_WAV_READER_H
#define AUDIO_EQUALIZER_WAV_READER_H

#include "audio_processor.h"

#ifdef __cplusplus
extern "C" {
#endif

    // WAV header structure
    typedef struct {
        char riff_header[4];
        uint32_t wav_size;
        char wave_header[4];
        char fmt_header[4];
        uint32_t fmt_chunk_size;
        uint16_t audio_format;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t byte_rate;
        uint16_t sample_alignment;
        uint16_t bit_depth;
        char data_header[4];
        uint32_t data_bytes;
    } wav_header_t;

    // WAV read/write functions
    audio_buffer_t* read_wav_file(const char* filename);
    int write_wav_file(const char* filename, audio_buffer_t* buffer);
    void free_audio_buffer(audio_buffer_t* buffer);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_EQUALIZER_WAV_READER_H