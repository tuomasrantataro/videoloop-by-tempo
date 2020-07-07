cmake_minimum_required(VERSION 3.0.0)
project(videoloop-by-tempo VERSION 0.1.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Multimedia REQUIRED)
find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(PkgConfig REQUIRED)

pkg_check_modules(libessentia REQUIRED IMPORTED_TARGET essentia)

add_executable(videoloop-by-tempo
    looprenderer.cpp
    vulkanwindow.cpp
    audiodatahandler.cpp
    audiodevice.cpp
    rhythmextractor.cpp
    gui.cpp
    main.cpp
)

target_link_libraries(videoloop-by-tempo Qt5::Widgets)
target_link_libraries(videoloop-by-tempo Qt5::Multimedia)
target_link_libraries(videoloop-by-tempo ${VULKAN_LIB_LIST})
target_link_libraries(videoloop-by-tempo PkgConfig::libessentia)
