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

static long get_file_size(FILE* file) {
    long current_pos = ftell(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, current_pos, SEEK_SET);
    return size;
}

audio_buffer_t* read_wav_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }

    long file_size = get_file_size(file);
    printf("Taille du fichier: %ld octets\n", file_size);

    // Structure pour l'en-tête RIFF
    struct {
        char riff_header[4];
        uint32_t wav_size;
        char wave_header[4];
    } riff_header;

    // Lire l'en-tête RIFF
    if (fread(&riff_header, sizeof(riff_header), 1, file) != 1) {
        fclose(file);
        return NULL;
    }

    // Vérifier l'en-tête RIFF
    if (strncmp(riff_header.riff_header, "RIFF", 4) != 0 ||
        strncmp(riff_header.wave_header, "WAVE", 4) != 0) {
        fclose(file);
        return NULL;
    }

    // Variables pour stocker le format audio
    uint16_t audio_format = 0;
    uint16_t num_channels = 0;
    uint32_t sample_rate = 0;
    uint16_t bit_depth = 0;
    uint32_t data_size = 0;

    // Lire les chunks jusqu'à trouver le chunk "data"
    char chunk_header[4];
    uint32_t chunk_size;

    long data_position = 0;

    while (fread(chunk_header, sizeof(chunk_header), 1, file) == 1) {
        if (fread(&chunk_size, sizeof(chunk_size), 1, file) != 1) {
            printf("Erreur de lecture de la taille du chunk\n");
            break;
        }

        printf("Chunk trouvé: %.4s, taille: %u\n", chunk_header, chunk_size);

        if (strncmp(chunk_header, "fmt ", 4) == 0) {
            // Lire le chunk format
            fread(&audio_format, 2, 1, file);
            fread(&num_channels, 2, 1, file);
            fread(&sample_rate, 4, 1, file);
            fseek(file, 4, SEEK_CUR);  // Ignorer byte_rate
            fseek(file, 2, SEEK_CUR);  // Ignorer block_align
            fread(&bit_depth, 2, 1, file);
            fseek(file, chunk_size - 16, SEEK_CUR);  // Sauter le reste du chunk
        }
        else if (strncmp(chunk_header, "data", 4) == 0) {
            data_position = ftell(file);
            if (chunk_size == 0xFFFFFFFF || chunk_size == 0) {
                // Si la taille est invalide, calculer la taille restante du fichier
                data_size = file_size - data_position;
            } else {
                data_size = chunk_size;
            }
            break;
        }
        else {
            // Sauter les chunks inconnus
            fseek(file, chunk_size, SEEK_CUR);
        }
    }

    printf("Format audio: %u\n", audio_format);
    printf("Canaux: %u\n", num_channels);
    printf("Taux d'échantillonnage: %u\n", sample_rate);
    printf("Bits par échantillon: %u\n", bit_depth);
    printf("Position des données: %ld\n", data_position);
    printf("Taille des données calculée: %u\n", data_size);

    if (data_size == 0 || audio_format != 1) {
        printf("Format audio non valide ou données introuvables\n");
        fclose(file);
        return NULL;
    }

    // Allouer la structure audio_buffer
    audio_buffer_t* buffer = (audio_buffer_t*)malloc(sizeof(audio_buffer_t));
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    // Calculer le nombre d'échantillons
    uint32_t bytes_per_sample = bit_depth / 8;
    uint32_t num_samples = data_size / (bytes_per_sample * num_channels);

    // Initialiser le buffer
    buffer->sample_rate = sample_rate;
    buffer->channels = num_channels;
    buffer->size = num_samples * num_channels;
    buffer->data = (float*)malloc(buffer->size * sizeof(float));

    if (!buffer->data) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    fseek(file, data_position, SEEK_SET);

    // Lire les données audio
    if (bit_depth == 16) {
        int16_t* temp = (int16_t*)malloc(data_size);
        if (!temp) {
            printf("Erreur d'allocation mémoire pour %u octets\n", data_size);
            free(buffer->data);
            free(buffer);
            fclose(file);
            return NULL;
        }

        size_t samples_read = fread(temp, 1, data_size, file);
        printf("Octets lus: %zu sur %u attendus\n", samples_read, data_size);

        // Convertir en float
        size_t num_samples = samples_read / sizeof(int16_t);
        buffer->size = num_samples;
        for (size_t i = 0; i < num_samples; i++) {
            buffer->data[i] = temp[i] / 32768.0f;
        }

        free(temp);
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