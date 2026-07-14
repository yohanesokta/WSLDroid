cmake_minimum_required(VERSION 3.16)
project(WSLDroidApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(WSLDroidApp WIN32
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    resources/app.rc
    resources.qrc
)

target_link_libraries(WSLDroidApp PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Output binary to ../bin
set_target_properties(WSLDroidApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin"
)

# Copy scripts directory to ../bin
add_custom_command(TARGET WSLDroidApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/scripts"
        "${CMAKE_SOURCE_DIR}/../bin/scripts"
    COMMENT "Copying scripts to bin directory"
)
