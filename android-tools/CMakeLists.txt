cmake_minimum_required(VERSION 3.16)
project(LowLatencyRDPEmulator LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Qt's MOC and UIC automatic processors
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt 6 configuration
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# FreeRDP configurations from vcpkg
find_package(FreeRDP CONFIG REQUIRED)
find_package(FreeRDP-Client CONFIG REQUIRED)
find_package(WinPR CONFIG REQUIRED)

add_executable(rdp_emulator
    resources/app.rc
    resources.qrc
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/phoneframe.h
    src/phoneframe.cpp
    src/rdpclient.h
    src/rdpclient.cpp
    src/rdpscreen.h
    src/rdpscreen.cpp
    src/toolbutton.h
    src/toolbutton.cpp
    src/readconfig.h
    src/readconfig.cpp
)

set_target_properties(rdp_emulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/../bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/../bin"
)

target_link_libraries(rdp_emulator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    freerdp
    freerdp-client
    winpr
)
