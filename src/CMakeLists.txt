# For Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INSTALL_MESSAGE NEVER)

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
    GIT_SUBMODULES ""
    BINARY_DIR ${PROJECT_SOURCE_DIR}/3rd_party/essentia
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/essentia
    CONFIGURE_COMMAND ./waf configure --ignore-algos=* --include-algos=RhythmExtractor2013,BeatTrackerMultiFeature,FrameCutter,NoiseAdder,Windowing,FFT,CartesianToPolar,OnsetDetection,HFC,Flux,MelBands,TriangularBands,TempoTapDegara,MovingAverage,IIR,AutoCorrelation,OnsetDetectionGlobal,Spectrum,Magnitude,ERBBands,TempoTapMaxAgreement,Scale --lightweight=fftw --build-static --prefix=./
    BUILD_COMMAND ./waf
    INSTALL_COMMAND ./waf install
)

ExternalProject_Add_Step(essentia
    add_run_permission_waf
    COMMAND chmod +x ./waf
    COMMENT "Setting permission to execute ./waf for building essentia"
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/3rd_party/essentia
)

SET(ESSENTIA_DIR ${PROJECT_SOURCE_DIR}/3rd_party/essentia)

ExternalProject_add(assimp
    GIT_REPOSITORY "https://github.com/assimp/assimp.git"
    GIT_TAG "v5.2.1"
    GIT_SHALLOW true
    GIT_PROGRESS true
    PREFIX ${PROJECT_SOURCE_DIR}/3rd_party/assimp
    SOURCE_DIR ${PREFIX}
    CMAKE_ARGS  -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF
                -DASSIMP_BUILD_OBJ_IMPORTER=ON
                -DASSIMP_NO_EXPORT=ON
                -DBUILD_SHARED_LIBS=OFF
                -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
                -DASSIMP_BUILD_TESTS=OFF
                -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/3rd_party/assimp
                -DASSIMP_INSTALL=ON
                -DASSIMP_INJECT_DEBUG_POSTFIX=0FF
)

SET(ASSIMP_DIR ${PROJECT_SOURCE_DIR}/3rd_party/assimp)

link_directories(${ASSIMP_DIR})

add_compile_options(-Wall)

add_library(AudioDevice audiodevice.cpp)
target_link_libraries(AudioDevice
    PUBLIC      Qt5::Multimedia)

add_library(DBManager dbmanager.cpp)
target_link_libraries(DBManager
    PRIVATE     Qt5::Sql)

add_library(ModelLoader modelloader.cpp)
add_dependencies(ModelLoader assimp)
target_include_directories(ModelLoader
    PUBLIC     ${ASSIMP_DIR}/include
)
target_link_directories(ModelLoader
    INTERFACE     ${ASSIMP_DIR}/lib
)
target_link_libraries(ModelLoader
    PUBLIC      Qt5::Core
                Qt5::Widgets
                libassimp.a)

add_library(FrameLoader frameloader.cpp)
target_link_libraries(FrameLoader
    PUBLIC      Qt5::Core
                Qt5::Gui)

add_library(OpenGLWidget2 openglwidget2.cpp)
target_link_libraries(OpenGLWidget2
    PUBLIC      Qt5::Core
                Qt5::Widgets
)

add_library(GraphicsWidget graphicswidget.cpp)
target_link_libraries(GraphicsWidget
    PUBLIC      ModelLoader
                FrameLoader
                OpenGLWidget2
                Qt5::Core
                Qt5::Widgets
)

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

add_library(Track track.cpp)
target_link_libraries(Track
    PUBLIC      Qt5::Core
                DBManager
                SpotifyWatcher
                PulseAudioWatcher
)

add_library(ScreenSaverInhibitor screensaverinhibitor.cpp)
target_link_libraries(ScreenSaverInhibitor
    PUBLIC      Qt5::Core
                Qt5::DBus)

add_library(MainWindow mainwindow.cpp)
add_dependencies(MainWindow RhythmExtractor)
target_link_libraries(MainWindow
    PUBLIC      Qt5::Widgets
                RhythmExtractor
                AudioDevice
                Settings
                Tempo
                Track
                OpenGLWidget2
                ModelLoader
                GraphicsWidget
                ScreenSaverInhibitor
)

add_executable(videoloop-by-tempo main.cpp)

#add_dependencies(videoloop-by-tempo assimp)
#target_link_directories(videoloop-by-tempo
#    INTERFACE     ${ASSIMP_DIR}/lib
#)
target_link_libraries(videoloop-by-tempo
                Qt5::Widgets
                libessentia.a
                libfftw3f.a
                #libassimp.a
                MainWindow
)
