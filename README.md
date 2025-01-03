# FFT Audio Processing Library

A C library for real-time Fast Fourier Transform (FFT) processing, designed for audio applications.

## Features

- FFT and inverse FFT implementations
- Window functions (Hanning)
- Magnitude spectrum computation
- Test suite with various signal types (sine waves, noise, frequency sweeps)

## Building

### Prerequisites

- CMake 3.24 or higher
- C compiler supporting C11 standard
- math.h library (usually included with compiler)

### Unix-like Systems (Linux, macOS)

```bash
# Clone repository
git clone https://github.com/yourusername/fft-audio.git
cd fft-audio

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make
```

## Windows (with Visual Studio)

### Clone repository
git clone https://github.com/yourusername/fft-audio.git
cd fft-audio

### Create build directory
mkdir build
cd build

### Configure project
cmake -G "Visual Studio 17 2022" ..

### Build (or open .sln in Visual Studio)
cmake --build . --config Release

## Testing

```bash
./fft_test
```

## Project Structure

```
fft-audio/
├── src/
│   └── core/
│       ├── fft.c
│       └── fft.h
├── test/
│   └── fft_test.c
└── CMakeLists.txt
```

## API Overview

- void fft(cplx buf[], int n): Forward FFT
- void ifft(cplx buf[], int n): Inverse FFT
- void apply_window(double* buffer, int size): Apply Hanning window
- void compute_magnitude_spectrum(cplx* fft_result, double* magnitude, int size): Calculate magnitude spectrum

## License
MIT License