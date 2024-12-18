//
// Created by Léo KRYS on 18/12/2024.
//

#ifndef AUDIO_EQUALIZER_FFT_H
#define AUDIO_EQUALIZER_FFT_H

#include <stdint.h>

#ifdef __cplusplus
    #include <complex>
    typedef std::complex<double> cplx;
#else
#include <complex.h>
typedef double _Complex cplx;
#endif

#ifdef __cplusplus
extern "C" {
#endif

    // FFT main functions
    void fft(cplx buf[], int n);
    void ifft(cplx buf[], int n);

    // Utils function
    void apply_window(double* buffer, int size);
    void compute_magnitude_spectrum(cplx* fft_result, double* magnitude, int size);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_EQUALIZER_FFT_H

