cmake_minimum_required(VERSION 3.0.0)
project(videoloop-by-tempo VERSION 0.1.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Multimedia REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(essentia REQUIRED IMPORTED_TARGET essentia)

add_executable(videoloop-by-tempo
    openglwidget.cpp
    audiodatahandler.cpp
    audiodevice.cpp
    rhythmextractor.cpp
    gui.cpp
    main.cpp
)

target_link_libraries(videoloop-by-tempo Qt5::Widgets)
target_link_libraries(videoloop-by-tempo Qt5::Multimedia)
target_link_libraries(videoloop-by-tempo PkgConfig::essentia)
