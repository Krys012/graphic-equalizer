//
// Created by Léo KRYS on 18/12/2024.
//

#include "wav_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verify_wav_header(wav_header_t* header) {
    if (strncmp(header->riff_header, "RIFF", 4) != 0) {
        printf("Erreur: En-tête RIFF invalide\n");
        return 0;
    }
    if (strncmp(header->wave_header, "WAVE", 4) != 0) {
        printf("Erreur: En-tête WAVE invalide\n");
        return 0;
    }
    if (strncmp(header->fmt_header, "fmt ", 4) != 0) {
        printf("Erreur: En-tête fmt invalide\n");
        return 0;
    }
    if (header->audio_format != 1) { // PCM = 1
        printf("Erreur: Format audio non supporté (non PCM)\n");
        return 0;
    }
    return 1;
}

audio_buffer_t* read_wav_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }

    audio_buffer_t* buffer = (audio_buffer_t*)malloc(sizeof(audio_buffer_t));
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    wav_header_t header;
    if (fread(&header, sizeof(wav_header_t), 1, file) != 1) {
        printf("Erreur: Lecture de l'en-tête WAV impossible\n");
        free(buffer);
        fclose(file);
        return NULL;
    }

    if (!verify_wav_header(&header)) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer->sample_rate = header.sample_rate;
    buffer->channels = header.num_channels;

    uint32_t bytes_per_sample = header.bit_depth / 8;
    buffer->size = header.data_bytes / bytes_per_sample;


    buffer->data = (float*)malloc(buffer->size * sizeof(float));
    if (!buffer->data) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    if (header.bit_depth == 16) {
        int16_t* temp_buffer = (int16_t*)malloc(header.data_bytes);
        if (!temp_buffer) {
            free(buffer->data);
            free(buffer);
            fclose(file);
            return NULL;
        }

        if (fread(temp_buffer, header.data_bytes, 1, file) != 1) {
            free(temp_buffer);
            free(buffer->data);
            free(buffer);
            fclose(file);
            return NULL;
        }

        for (uint32_t i = 0; i < buffer->size; i++) {
            buffer->data[i] = temp_buffer[i] / 32768.0f;
        }

        free(temp_buffer);
    }
    else if (header.bit_depth == 24) {
        uint8_t* temp_buffer = (uint8_t*)malloc(header.data_bytes);
        if (!temp_buffer) {
            free(buffer->data);
            free(buffer);
            fclose(file);
            return NULL;
        }

        if (fread(temp_buffer, header.data_bytes, 1, file) != 1) {
            free(temp_buffer);
            free(buffer->data);
            free(buffer);
            fclose(file);
            return NULL;
        }

        for (uint32_t i = 0; i < buffer->size; i++) {
            int32_t sample = (temp_buffer[i*3] << 8) |
                            (temp_buffer[i*3 + 1] << 16) |
                            (temp_buffer[i*3 + 2] << 24);
            sample >>= 8; // Ajuster à 24-bit
            buffer->data[i] = sample / 8388608.0f; // 2^23
        }

        free(temp_buffer);
    }
    else {
        printf("Erreur: Profondeur de bits non supportée: %d\n", header.bit_depth);
        free(buffer->data);
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

int write_wav_file(const char* filename, audio_buffer_t* buffer) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Erreur: Impossible de créer le fichier %s\n", filename);
        return 0;
    }

    wav_header_t header;
    memcpy(header.riff_header, "RIFF", 4);
    memcpy(header.wave_header, "WAVE", 4);
    memcpy(header.fmt_header, "fmt ", 4);
    memcpy(header.data_header, "data", 4);

    header.fmt_chunk_size = 16;
    header.audio_format = 1; // PCM
    header.num_channels = buffer->channels;
    header.sample_rate = buffer->sample_rate;
    header.bit_depth = 16; // On écrit toujours en 16 bits
    header.sample_alignment = header.num_channels * (header.bit_depth / 8);
    header.byte_rate = header.sample_rate * header.sample_alignment;
    header.data_bytes = buffer->size * (header.bit_depth / 8);
    header.wav_size = 36 + header.data_bytes;

    if (fwrite(&header, sizeof(wav_header_t), 1, file) != 1) {
        fclose(file);
        return 0;
    }

    int16_t* temp_buffer = (int16_t*)malloc(buffer->size * sizeof(int16_t));
    if (!temp_buffer) {
        fclose(file);
        return 0;
    }

    // Convertir float en int16
    for (uint32_t i = 0; i < buffer->size; i++) {
        float sample = buffer->data[i];
        // Limiter entre -1 et 1
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        // Convertir en int16
        temp_buffer[i] = (int16_t)(sample * 32767.0f);
    }

    size_t written = fwrite(temp_buffer, sizeof(int16_t), buffer->size, file);
    free(temp_buffer);
    fclose(file);

    return (written == buffer->size);
}

void free_audio_buffer(audio_buffer_t* buffer) {
    if (buffer) {
        if (buffer->data) {
            free(buffer->data);
        }
        free(buffer);
    }
}