# FreeRDP Android Client

This project is a high-performance, modular FreeRDP 3.x client built with Qt 6.11.1 and compiled under MinGW-w64 (GCC 13.1.0). It is designed to display a Weston RDP backend running inside Windows Subsystem for Linux (WSL) as a phone-like Android Virtual Device (AVD) emulator skin. It features extremely low latency and interactive mouse and keyboard controls.

---

## Architecture and Design

The application is structured into key components to separate concerns and ensure clean, extensible code:

### 1. RdpClient (Network Thread)
* **Threading**: Runs the blocking FreeRDP connection and event loop in a background QThread to ensure that the GUI thread remains completely fluid and responsive (60+ FPS).
* **WinPR Integration**: Uses Windows Portable Runtime (WinPR) synchronization (HANDLE, WaitForMultipleObjects) to combine FreeRDP's socket descriptors with a custom local input event queue. When a user performs mouse or keyboard actions, the thread is instantly woken up to transmit packets.
* **Decoders**: Leverages FreeRDP's Software GDI subsystem to decode and paint remote frames directly into a linear framebuffer (primary_buffer).

### 2. RdpScreen (Display and Input Capture)
* **Rendering**: Reads the shared decoded framebuffer under a thread-safe lock and renders it to the screen using hardware-accelerated QPainter with rounded clipping.
* **Coordinate Mapping**: Dynamically maps local widget mouse positions to RDP absolute pixel coordinates based on the remote desktop resolution.
* **Keyboard Translators**: Captures native Windows hardware keyboard scancodes directly. RDP protocol operates on IBM PC XT/AT physical scancodes, making Windows native scancodes compatible with RDP keyboard packets without manual layout mapping tables.

### 3. ToolButton (Vector Icon Renderers)
* **Custom Vectors**: Self-contained, resolution-agnostic buttons that draw high-fidelity AVD icons (Power, Volume, Screenshot, Home, Back, Recents, and Help) programmatically. This removes the need for external PNG or SVG asset files, keeping the build standalone and sharp on high-DPI (Retina) screens.

### 4. MainWindow (AVD Shell)
* **AVD Theme**: Structures the mobile phone chassis layout. 
* **Window Attributes**: Uses frameless and transparent window attributes (translucent background) to display only the mobile phone chassis and floating vertical toolbar on the desktop.
* **Dragging**: Permits dragging the bezel of the phone chassis to reposition the floating emulator window anywhere on the desktop.

---

## Low-Latency Optimizations

This client is specifically tuned for local RDP performance (WSL to Host) to minimize delay:
1. **Fast-Path Input/Output**: Bypasses traditional slow-path graphics and input packet marshalling for raw TCP/UDP transmission.
2. **GDI Frame Copying**: Employs line-by-line memory copying of the decoded framebuffer directly, avoiding intermediate scaling or conversion overhead before rendering.
3. **No-Delay Synchronization**: The thread wakes up immediately on mouse movement or key press, resulting in sub-millisecond input relay.

---

## How to Build and Run on Windows

### Prerequisites
1. **Qt 6 (MinGW)**: Download and install the Qt Online Installer. Select and install Qt 6.x for MinGW-w64 (e.g., Qt 6.11 MinGW 64-bit).
2. **MinGW, CMake, and Ninja**: These build tools can be installed directly through the Qt Maintenance Tool under the "Developer and Designer Tools" section.
3. **vcpkg**: Install Microsoft's C++ package manager and the required FreeRDP/WinPR libraries:
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg install freerdp3:x64-windows winpr3:x64-windows
   ```

### Building
A helper script `build.ps1` is provided to automate the CMake configuration and Ninja build process.

1. Open PowerShell in the root directory of this repository.
2. Run the build script, passing the paths to your Qt and vcpkg installations. Adjust the paths according to where you installed them:
   ```powershell
   .\build.ps1 -QtPath "C:\Qt\6.11.1\mingw_64" -QtToolsPath "C:\Qt\Tools" -VcpkgPath "C:\vcpkg"
   ```
   *(If you omit the parameters, the script will attempt to use default common paths).*

3. Run the application:
   ```powershell
   .\build\rdp_emulator.exe --host=127.0.0.1 --port=3389 --width=393 --height=852
   ```

---

## Command-Line Arguments

The application accepts the following command-line flags:
* `--host=<ip>` : Specifies the target host IP address (Default: 127.0.0.1).
* `--port=<port>` : Specifies the target server port (Default: 3389).
* `--width=<width>` : Specifies the desktop resolution width to negotiate with RDP (Default: 393).
* `--height=<height>` : Specifies the desktop resolution height to negotiate with RDP (Default: 852).

---

## Interacting with the Emulator

* **Repositioning**: Left-click and drag the outer black phone bezel to move the floating window across your desktop.
* **Closing**: Press the Escape key or click the Power button on the right-hand vertical toolbar to terminate the application.
* **ADB Commands**: The toolbar buttons trigger detached background ADB processes to control the remote device (e.g., volume control, back, home, and recents).
