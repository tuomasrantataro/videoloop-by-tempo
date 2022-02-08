# For Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)

find_package(Qt5 COMPONENTS Core Concurrent Widgets Multimedia DBus Sql REQUIRED)
find_package(PulseAudio REQUIRED)

include(ExternalProject)

ExternalProject_add(essentia
    GIT_REPOSITORY "https://github.com/MTG/essentia.git"
    # Use the newest tag when this change was made to keep from updating unnecessarily.
    # Stable release is really old, and betas also old. But this seems to work.
    GIT_TAG "v2.1_beta5"
    GIT_SHALLOW true
    GIT_PROGRESS true
    BINARY_DIR ../../3rd_party/essentia
    DOWNLOAD_DIR ../../3rd_party/essentia
    SOURCE_DIR ../../3rd_party/essentia
    CONFIGURE_COMMAND ./waf configure --lightweight=fftw --build-static --prefix=./
    BUILD_COMMAND ./waf
    INSTALL_COMMAND ./waf install
)

ExternalProject_Add_Step(essentia
    add_run_permission_waf
    COMMAND chmod +x ./waf
    COMMENT "Setting permission to execute ./waf for building essentia"
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY ../../3rd_party/essentia
)

SET(ESSENTIA_DIR ${PROJECT_SOURCE_DIR}/3rd_party/essentia)


add_compile_options(-Wall)

add_library(AudioDevice audiodevice.cpp)
target_link_libraries(AudioDevice
    PUBLIC      Qt5::Multimedia)

add_library(DBManager dbmanager.cpp)
target_link_libraries(DBManager
    PRIVATE     Qt5::Sql)

add_library(OpenGLWidget openglwidget.cpp)
target_link_libraries(OpenGLWidget
    PUBLIC      Qt5::Widgets)

add_library(PulseAudioWatcher pulseaudiowatcher.cpp pulseaudiowatcher.h)
target_link_libraries(PulseAudioWatcher
    PUBLIC      Qt5::Core
    PRIVATE     pulse
)

add_library(RhythmExtractor rhythmextractor.cpp)
add_dependencies(RhythmExtractor essentia)
target_link_directories(RhythmExtractor
    INTERFACE   ${ESSENTIA_DIR}/lib
)
target_include_directories(RhythmExtractor
    PRIVATE     ${ESSENTIA_DIR}/include
)   
target_link_libraries(RhythmExtractor
    PUBLIC      Qt5::Core
    PRIVATE     libfftw3f.a
                libessentia.a
)

add_library(SpotifyWatcher spotifywatcher.cpp spotifywatcher.h)
target_link_libraries(SpotifyWatcher
    PUBLIC      Qt5::Core
                Qt5::DBus
)

add_library(Settings settings.cpp)
target_link_libraries(Settings
    PUBLIC      Qt5::Core
)

add_library(Tempo tempo.cpp)
target_link_libraries(Tempo
    PUBLIC      Qt5::Core
)

add_library(GUI gui.cpp)
add_dependencies(GUI RhythmExtractor)
target_link_libraries(GUI
    PUBLIC      Qt5::Widgets
                RhythmExtractor
                AudioDevice
                DBManager
                OpenGLWidget
                SpotifyWatcher
                PulseAudioWatcher
                Settings
                Tempo
)

add_executable(videoloop-by-tempo main.cpp)

add_dependencies(videoloop-by-tempo GUI)

target_link_libraries(videoloop-by-tempo
                Qt5::Widgets
                libessentia.a
                libfftw3f.a
                GUI
)
