cmake_minimum_required(VERSION 3.0.0)
project(videoloop-by-tempo VERSION 1.0.0)

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

install(TARGETS videoloop-by-tempo DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/release)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release/")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/frames/" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release/frames/")
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/README.md" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/install.sh" DESTINATION "${PROJECT_SOURCE_DIR}/release/")

set(CPACK_PACKAGE_VENDOR "Tuomas Rantataro")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Adjust the speed of your video loop according to tempo of sound output")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")

include(CPack)