cmake_minimum_required(VERSION 3.24)
project(audio_equalizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configuration de Qt
find_package(Qt6 COMPONENTS
        Core
        Widgets
        Multimedia
        REQUIRED
)

# Sources C
set(CORE_SOURCES
        src/core/fft.c
        src/core/wav_reader.c
        src/core/audio_processor.c
)

# Sources C++
set(GUI_SOURCES
        src/gui/main_window.cpp
        src/gui/equalizer_widget.cpp
        src/gui/spectrum_display.cpp
        src/gui/audio_backend.cpp
        src/main.cpp
)

# Headers
set(HEADERS
        src/core/fft.h
        src/core/wav_reader.h
        src/core/audio_processor.h
        src/gui/main_window.h
        src/gui/equalizer_widget.h
        src/gui/spectrum_display.h
        src/gui/audio_backend.h
)

# Créer une bibliothèque pour le code C
add_library(core_audio STATIC ${CORE_SOURCES})
target_include_directories(core_audio PUBLIC src/core)

# Créer l'exécutable principal
add_executable(${PROJECT_NAME}
        ${GUI_SOURCES}
        ${HEADERS}
)

# Lier les bibliothèques
target_link_libraries(${PROJECT_NAME} PRIVATE
        core_audio
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
)

# Ajouter la bibliothèque math si nécessaire
if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# Configuration pour le déploiement
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

# Pour le développement, activer les warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    target_compile_options(core_audio PRIVATE -Wall -Wextra)
endif()