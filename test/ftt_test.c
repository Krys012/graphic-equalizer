//
// Created by Léo KRYS on 31/12/2024.
//
#include "fft.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define COMPLEXITY_TEST_SIZE 2048
#define NUM_TESTS 1000
#define PI 3.14159265358979323846
#define TEST_SIZE 256

void test_signal_1() {
    // Pure sine wave at 440 Hz
    cplx signal[TEST_SIZE];
    double freq = 440.0;
    double samplingRate = 44100.0;

    for (int i = 0; i < TEST_SIZE; i++) {
        double t = i / samplingRate;
        signal[i] = sin(2.0 * PI * freq * t);
    }

    printf("Test 1 - Pure sine wave 440 Hz:\n");
    printf("First samples: %.3f, %.3f, %.3f, %.3f\n",
           creal(signal[0]), creal(signal[1]), creal(signal[2]), creal(signal[3]));
}

void test_signal_2() {
    // Complex signal: 440 Hz + 880 Hz
    cplx signal[TEST_SIZE];
    double freq1 = 440.0, freq2 = 880.0;
    double samplingRate = 44100.0;

    for (int i = 0; i < TEST_SIZE; i++) {
        double t = i / samplingRate;
        signal[i] = 0.7 * sin(2.0 * PI * freq1 * t) +
                    0.3 * sin(2.0 * PI * freq2 * t);
    }

    printf("Test 2 - Sum of two sine waves:\n");
    printf("First samples: %.3f, %.3f, %.3f, %.3f\n",
           creal(signal[0]), creal(signal[1]), creal(signal[2]), creal(signal[3]));
}

void test_signal_3() {
    // Signal with noise
    cplx signal[TEST_SIZE];
    double freq = 440.0;
    double samplingRate = 44100.0;

    for (int i = 0; i < TEST_SIZE; i++) {
        double t = i / samplingRate;
        double noise = ((double)rand() / RAND_MAX) * 0.1;  // 10% noise
        signal[i] = sin(2.0 * PI * freq * t) + noise;
    }

    printf("Test 3 - Sine wave with noise:\n");
    printf("First samples: %.3f, %.3f, %.3f, %.3f\n",
           creal(signal[0]), creal(signal[1]), creal(signal[2]), creal(signal[3]));
}

void test_signal_4() {
    // Frequency sweep
    cplx signal[TEST_SIZE];
    double freqStart = 200.0, freqEnd = 2000.0;
    double samplingRate = 44100.0;

    for (int i = 0; i < TEST_SIZE; i++) {
        double t = i / samplingRate;
        double freq = freqStart + (freqEnd - freqStart) * i / TEST_SIZE;
        signal[i] = sin(2.0 * PI * freq * t);
    }

    printf("Test 4 - Frequency sweep:\n");
    printf("First samples: %.3f, %.3f, %.3f, %.3f\n",
           creal(signal[0]), creal(signal[1]), creal(signal[2]), creal(signal[3]));
}

void test_signal_5() {
    // Square wave
    cplx signal[TEST_SIZE];
    double freq = 440.0;
    double samplingRate = 44100.0;

    for (int i = 0; i < TEST_SIZE; i++) {
        double t = i / samplingRate;
        signal[i] = (sin(2.0 * PI * freq * t) > 0) ? 1.0 : -1.0;
    }

    printf("Test 5 - Square wave:\n");
    printf("First samples: %.3f, %.3f, %.3f, %.3f\n",
           creal(signal[0]), creal(signal[1]), creal(signal[2]), creal(signal[3]));
}

void verify_fft(cplx signal[], int size) {
    cplx original[TEST_SIZE];
    for (int i = 0; i < size; i++) {
        original[i] = signal[i];
    }

    fft(signal, size);
    ifft(signal, size);

    double maxError = 0.0;
    for (int i = 0; i < size; i++) {
        double error = cabs(signal[i] - original[i]);
        if (error > maxError) maxError = error;
    }

    printf("Maximum error after FFT/IFFT: %.6e\n", maxError);
}

// Time complexity test
void test_temporal_complexity() {
    clock_t start, end;
    double cpu_time_used;

    printf("\nTime complexity test:\n");

    // Test different sizes
    for(int n = 64; n <= COMPLEXITY_TEST_SIZE; n *= 2) {
        cplx* signal = malloc(n * sizeof(cplx));

        // Signal generation
        for(int i = 0; i < n; i++) {
            signal[i] = rand() / (double)RAND_MAX;
        }

        // Time measurement
        start = clock();
        for(int iter = 0; iter < NUM_TESTS; iter++) {
            fft(signal, n);
        }
        end = clock();

        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("N = %d: %f seconds (N*log(N) = %f)\n",
               n, cpu_time_used, n * log2(n));

        free(signal);
    }
}

// Space complexity test
void test_spatial_complexity() {
    printf("\nSpace complexity test:\n");

    for(int n = 64; n <= COMPLEXITY_TEST_SIZE; n *= 2) {
        size_t baseline = 0;
        size_t peak = 0;

        // Initial memory measurement
        baseline = sizeof(cplx) * n;  // Input buffer

        // Peak memory measurement
        peak = baseline +
               2 * (sizeof(cplx) * n/2) +  // Temporary arrays
               sizeof(cplx) * log2(n);     // Recursion stack

        printf("N = %d:\n", n);
        printf("  Baseline memory: %zu bytes\n", baseline);
        printf("  Peak memory: %zu bytes\n", peak);
        printf("  Ratio: %.2fx\n", (double)peak/baseline);
    }
}

int main() {
    test_signal_1();
    test_signal_2();
    test_signal_3();
    test_signal_4();
    test_signal_5();

    test_temporal_complexity();
    test_spatial_complexity();

    return 0;
}