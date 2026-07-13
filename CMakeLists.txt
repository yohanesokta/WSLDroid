cmake_minimum_required(VERSION 3.22)
project(OctaDroid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Win32 GUI app configuration (prevents terminal popups on launch)
set(WIN32_EXECUTABLE ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core Gui OpenGLWidgets OpenGL Network REQUIRED)

# Source files grouped by architecture modules
set(SOURCES
    src/main.cpp
    
    # Core
    src/core/WSLManager.h
    src/core/WSLManager.cpp
    src/core/KernelManager.h
    src/core/KernelManager.cpp
    
    # Weston
    src/weston/WestonService.h
    src/weston/WestonService.cpp
    
    # Waydroid
    src/waydroid/WaydroidManager.h
    src/waydroid/WaydroidManager.cpp
    
    # Graphics
    src/graphics/FrameQueue.h
    src/graphics/FreeRDPClient.h
    src/graphics/FreeRDPClient.cpp
    
    # ADB
    src/adb/ADBManager.h
    src/adb/ADBManager.cpp
    
    # Styles
    src/styles/StyleHelper.h
    src/styles/StyleHelper.cpp
    
    # UI components
    src/ui/WindowManager.h
    src/ui/WindowManager.cpp
    src/ui/panels/SidebarPanel.h
    src/ui/panels/SidebarPanel.cpp
    src/ui/panels/StatusBarPanel.h
    src/ui/panels/StatusBarPanel.cpp
    src/ui/toolbar/TopToolbar.h
    src/ui/toolbar/TopToolbar.cpp
    src/ui/toolbar/EmulatorToolbar.h
    src/ui/toolbar/EmulatorToolbar.cpp
    src/ui/widgets/AndroidRenderWidget.h
    src/ui/widgets/AndroidRenderWidget.cpp
    src/ui/widgets/EmulatorWindow.h
    src/ui/widgets/EmulatorWindow.cpp
)

add_executable(OctaDroid WIN32 ${SOURCES})

target_link_libraries(OctaDroid PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets
    Qt6::OpenGL
    Qt6::Network
    freerdp3
    freerdp-client3
    winpr3
)

target_include_directories(OctaDroid PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    "D:/Programs/MSYS/mingw64/include/freerdp3"
    "D:/Programs/MSYS/mingw64/include/winpr3"
)

target_link_directories(OctaDroid PRIVATE
    "D:/Programs/MSYS/mingw64/lib"
)
