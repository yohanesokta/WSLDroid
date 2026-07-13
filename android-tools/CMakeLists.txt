cmake_minimum_required(VERSION 3.16)
project(LowLatencyRDPEmulator LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Qt's MOC and UIC automatic processors
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Qt 6 configuration
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# FreeRDP configurations from vcpkg
find_package(FreeRDP CONFIG REQUIRED)
find_package(FreeRDP-Client CONFIG REQUIRED)
find_package(WinPR CONFIG REQUIRED)

add_executable(rdp_emulator
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
)

target_link_libraries(rdp_emulator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    freerdp
    freerdp-client
    winpr
)
