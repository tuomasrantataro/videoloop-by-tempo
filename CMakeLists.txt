cmake_minimum_required(VERSION 3.14.0)
project( videoloop-by-tempo
         VERSION 1.1.0
         DESCRIPTION "Show video loop which changes its playback speed according to computer's sound output"
         HOMEPAGE_URL "https://github.com/tuomasrantataro/videoloop-by-tempo"
         LANGUAGES CXX )

# For Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)

find_package(Qt5 COMPONENTS Widgets Multimedia DBus Sql REQUIRED)
find_package(PulseAudio REQUIRED)
# For Essentia
find_package(Eigen3 REQUIRED)

include(ExternalProject)

ExternalProject_add(essentia
    GIT_REPOSITORY "https://github.com/MTG/essentia.git"
    # Use the newest commit when this change was made to keep from updating unnecessarily.
    # Stable release is really old, and betas also old. This seems to work.
    GIT_TAG "ee019f767570bae9946c6a58ea7231569be8a117"
    GIT_SHALLOW true
    GIT_PROGRESS true
    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/essentia
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/essentia
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/essentia
    CONFIGURE_COMMAND ./waf configure --lightweight=fftw --static-dependencies --build-static --prefix=./
    BUILD_COMMAND ./waf
    INSTALL_COMMAND ./waf install
)

ExternalProject_Add_Step(essentia
    add_run_permission_waf
    COMMAND chmod +x ./waf
    COMMENT "Setting permission to execute ./waf for building essentia"
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/essentia
)

SET(ESSENTIA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/essentia)

include_directories(${ESSENTIA_DIR}/include)
link_directories(${ESSENTIA_DIR}/lib)

add_executable(videoloop-by-tempo)

add_dependencies(videoloop-by-tempo essentia)

target_sources(videoloop-by-tempo
    PRIVATE     src/spotifywatcher.cpp
                src/pulseaudiowatcher.cpp
                src/dbmanager.cpp
                src/openglwidget.cpp
                src/audiodevice.cpp
                src/rhythmextractor.cpp
                src/gui.cpp
                src/main.cpp
)
target_compile_options(videoloop-by-tempo PRIVATE -Wall)

target_link_directories(videoloop-by-tempo
    INTERFACE   ${ESSENTIA_DIR}/lib
)

target_link_libraries(videoloop-by-tempo
    PRIVATE     Qt5::Widgets
                Qt5::Multimedia
                Qt5::DBus
                Qt5::Sql
                pulse
                Eigen3::Eigen
                libessentia.a
                libfftw3f.a)

# Create a folder which has only the relevant files for using the program
install(TARGETS videoloop-by-tempo DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/frames/" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release/frames/")
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/README.md" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/icon_256px.png" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
