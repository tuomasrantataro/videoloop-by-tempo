cmake_minimum_required(VERSION 3.14.0)
project( videoloop-by-tempo
         VERSION 1.1.0
         DESCRIPTION "Show video loop which changes its playback speed according to computer's sound output"
         HOMEPAGE_URL "https://github.com/tuomasrantataro/videoloop-by-tempo"
         LANGUAGES CXX )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_subdirectory(src)


# Create a folder which has only the relevant files for using the program
install(TARGETS videoloop-by-tempo DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/frames/" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/release/frames/")
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/README.md" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
install(FILES "${PROJECT_SOURCE_DIR}/icon_256px.png" DESTINATION "${PROJECT_SOURCE_DIR}/release/")
