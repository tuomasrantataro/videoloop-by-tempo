cmake_minimum_required(VERSION 3.14.0)
project( videoloop-by-tempo
         VERSION 1.0.0
         DESCRIPTION "Show video loop which changes its playback speed according to computer's sound output"
         HOMEPAGE_URL "https://github.com/tuomasrantataro/videoloop-by-tempo"
         LANGUAGES CXX )

# For Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

add_compile_definitions(
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)

find_package(Qt5 COMPONENTS Widgets Multimedia REQUIRED)

## Importing essentia. Configured with ./waf configure --lightweight=fftw --build-static
add_library(essentia STATIC IMPORTED)
set_target_properties(essentia PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external/essentia/build/src/libessentia.a")
target_link_directories(essentia INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/external/essentia/packaging/debian_3rdparty/lib")
target_link_libraries(essentia
    INTERFACE   libfftw3f.a)

add_executable(videoloop-by-tempo)
target_sources(videoloop-by-tempo
    PRIVATE src/openglwidget.cpp
            src/audiodatahandler.cpp
            src/audiodevice.cpp
            src/rhythmextractor.cpp
            src/gui.cpp
            src/main.cpp
)
target_compile_options(videoloop-by-tempo PRIVATE -fPIC -Wall -pedantic)
target_include_directories(videoloop-by-tempo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external/essentia/packaging/debian_3rdparty/include/eigen3")
target_link_libraries(videoloop-by-tempo
    PRIVATE     Qt5::Widgets
                Qt5::Multimedia
                essentia)

# Create a folder which has only the relevant files for using the program
install(TARGETS videoloop-by-tempo DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/frames/" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release/frames/")
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/README.md" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/install.sh" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/icon_256px.png" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
