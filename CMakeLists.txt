cmake_minimum_required(VERSION 3.24)
project(audio_equalizer)

set(CMAKE_C_STANDARD 11)

add_executable(fft_test
        test/ftt_test.c
        src/core/fft.c
        src/core/fft.c
)

target_include_directories(fft_test PUBLIC src/core)

if(UNIX)
    target_link_libraries(fft_test PRIVATE m)
endif()