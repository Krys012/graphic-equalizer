//
// Created by Léo KRYS on 18/12/2024.
//

#include "fft.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

void fft(cplx buf[], int n) {
    if (n <= 1) return;

    cplx even[n/2];
    cplx odd[n/2];
    for (int i = 0; i < n/2; i++) {
        even[i] = buf[2*i];
        odd[i] = buf[2*i+1];
    }

    fft(even, n/2);
    fft(odd, n/2);

    for (int k = 0; k < n/2; k++) {
        cplx t = cexp(-2.0 * PI * I * k / n) * odd[k];
        buf[k] = even[k] + t;
        buf[k + n/2] = even[k] - t;
    }
}

void ifft(cplx buf[], int n) {

    for (int i = 0; i < n; i++) {
        buf[i] = conj(buf[i]);
    }

    fft(buf, n);

    for (int i = 0; i < n; i++) {
        buf[i] = conj(buf[i]) / n;
    }
}

void apply_window(double* buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] *= 0.5 * (1 - cos(2*PI*i / (size-1)));
    }
}

void compute_magnitude_spectrum(cplx* fft_result, double* magnitude, int size) {
    for (int i = 0; i < size; i++) {
        magnitude[i] = cabs(fft_result[i]);
    }
}
