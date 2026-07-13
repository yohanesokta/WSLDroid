param (
    [string]$QtPath = "C:\Qt\6.11.1\mingw_64",
    [string]$QtToolsPath = "C:\Qt\Tools",
    [string]$VcpkgPath = "C:\vcpkg"
)

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host " Building Custom RDP Emulator Client     " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# 1. Resolve Compiler and Tool paths
# Usually, Qt installs MinGW inside a folder like mingw1310_64 inside Tools.
# We will search for g++.exe to find the exact MinGW bin directory.
$MinGWBinDir = ""
$gccPath = Get-ChildItem -Path $QtToolsPath -Filter "g++.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
if ($gccPath) {
    $MinGWBinDir = $gccPath.DirectoryName
    Write-Host "[+] Found MinGW at: $MinGWBinDir" -ForegroundColor Green
} else {
    Write-Host "[-] Could not find g++.exe in $QtToolsPath" -ForegroundColor Red
    exit 1
}

$NinjaDir = ""
$ninjaPath = Get-ChildItem -Path $QtToolsPath -Filter "ninja.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
if ($ninjaPath) {
    $NinjaDir = $ninjaPath.DirectoryName
    Write-Host "[+] Found Ninja at: $NinjaDir" -ForegroundColor Green
} else {
    Write-Host "[-] Could not find ninja.exe in $QtToolsPath" -ForegroundColor Red
    exit 1
}

$VcpkgPrefix = Join-Path $VcpkgPath "installed\x64-windows"
if (!(Test-Path $VcpkgPrefix)) {
    Write-Host "[-] vcpkg installed\x64-windows path not found at $VcpkgPrefix" -ForegroundColor Red
    Write-Host "[-] Please run: .\vcpkg install freerdp3:x64-windows winpr3:x64-windows" -ForegroundColor Yellow
    exit 1
}

# 2. Add tools to PATH
$env:PATH = "$MinGWBinDir;$NinjaDir;" + $env:PATH

# 3. Configure CMake
$CxxCompiler = Join-Path $MinGWBinDir "g++.exe"
$CCompiler = Join-Path $MinGWBinDir "gcc.exe"
$NinjaExe = Join-Path $NinjaDir "ninja.exe"
$PrefixPath = "$QtPath;$VcpkgPrefix"

# Fix path slashes for CMake
$CxxCompiler = $CxxCompiler -replace "\\", "/"
$CCompiler = $CCompiler -replace "\\", "/"
$NinjaExe = $NinjaExe -replace "\\", "/"
$PrefixPath = $PrefixPath -replace "\\", "/"

Write-Host "`n[+] Configuring CMake..." -ForegroundColor Cyan
cmake -B build -G Ninja `
    -DCMAKE_CXX_COMPILER="$CxxCompiler" `
    -DCMAKE_C_COMPILER="$CCompiler" `
    -DCMAKE_MAKE_PROGRAM="$NinjaExe" `
    -DCMAKE_PREFIX_PATH="$PrefixPath"

if ($LASTEXITCODE -ne 0) {
    Write-Host "[-] CMake configuration failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 4. Build
Write-Host "`n[+] Building Project..." -ForegroundColor Cyan
cmake --build build

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n[+] Build successful!" -ForegroundColor Green
    Write-Host "You can run the emulator using:" -ForegroundColor Yellow
    Write-Host ".\build\rdp_emulator.exe --host=127.0.0.1 --port=3389 --width=393 --height=852" -ForegroundColor White
} else {
    Write-Host "`n[-] Build failed!" -ForegroundColor Red
}
