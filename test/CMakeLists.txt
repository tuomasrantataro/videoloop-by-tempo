# For Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)

find_package(Qt5 COMPONENTS Core Concurrent Widgets Multimedia DBus Sql Test REQUIRED)

include_directories(../src)

add_executable(tst_settings tst_settings.cpp)
target_link_libraries(tst_settings
                Qt5::Test
                Qt5::Core
                Settings
                
)

add_executable(tst_tempo tst_tempo.cpp)
target_link_libraries(tst_tempo
                Qt5::Test
                Qt5::Core
                Tempo
)