param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

# Convert to absolute path
if (-not (Test-Path $BuildDir)) {
    Write-Error "Build directory '$BuildDir' does not exist."
}
$BuildPath = (Resolve-Path $BuildDir).Path
$ExePath = Join-Path $BuildPath "emulator.exe"

if (-not (Test-Path $ExePath)) {
    Write-Error "Could not find emulator.exe in $BuildPath. Make sure to compile the project first."
}

Write-Host "Found emulator.exe. Deploying Qt dependencies..."

# 1. Run windeployqt
$WindeployqtPath = "D:\Programs\MSYS\mingw64\bin\windeployqt6.exe"
if (-not (Test-Path $WindeployqtPath)) {
    $WindeployqtPath = "D:\Programs\MSYS\mingw64\bin\windeployqt-qt6.exe"
}
if (-not (Test-Path $WindeployqtPath)) {
    $WindeployqtPath = "D:\Programs\MSYS\mingw64\bin\windeployqt.exe"
}
if (-not (Test-Path $WindeployqtPath)) {
    $WindeployqtPath = "D:\Programs\MSYS\mingw64\share\qt6\bin\windeployqt.exe"
}

if (Test-Path $WindeployqtPath) {
    Write-Host "Running windeployqt from $WindeployqtPath..."
    # Set path environment to include MSYS2 bin to avoid library load errors during deployment
    $OldPath = $env:PATH
    $env:PATH = "D:\Programs\MSYS\mingw64\bin;D:\Programs\MSYS\usr\bin;$env:PATH"
    & $WindeployqtPath --no-compiler-runtime --dir $BuildPath $ExePath
    $env:PATH = $OldPath
} else {
    Write-Warning "windeployqt.exe not found. You might need to install mingw-w64-x86_64-qt6-base."
}

# 2. Deploy non-Qt dependencies using ldd
Write-Host "Analyzing non-Qt DLL dependencies using MSYS2 ldd..."
# Convert Windows path of exe to MSYS unix path for ldd
# e.g., C:\Users\... -> /c/Users/...
$UnixPath = $ExePath.Replace('\', '/').Replace('C:', '/c').Replace('c:', '/c').Replace('D:', '/d').Replace('d:', '/d')

$LddPath = "D:\Programs\MSYS\usr\bin\bash.exe"
if (Test-Path $LddPath) {
    $LddOutput = & $LddPath -lc "export PATH=/mingw64/bin:`$PATH; ldd '$UnixPath'"
    
    $MsysBin = "D:\Programs\MSYS\mingw64\bin"
    foreach ($Line in $LddOutput) {
        # Match lines like: libfreerdp3.dll => /mingw64/bin/libfreerdp3.dll (0x...)
        if ($Line -match "=>\s+(/mingw64/bin/[^\s]+)") {
            $UnixDllPath = $Matches[1]
            $DllName = Split-Path $UnixDllPath -Leaf
            $SrcPath = Join-Path $MsysBin $DllName
            $DestPath = Join-Path $BuildPath $DllName
            
            if (Test-Path $SrcPath) {
                if (-not (Test-Path $DestPath)) {
                    Write-Host "Copying $DllName to build directory..."
                    Copy-Item $SrcPath $DestPath
                }
            }
        }
    }
    Write-Host "Dependency deployment completed successfully!"
} else {
    Write-Warning "MSYS2 bash not found at $LddPath. Cannot automate non-Qt DLL deployment."
}
