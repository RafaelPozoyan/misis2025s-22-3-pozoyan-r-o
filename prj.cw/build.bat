@echo off
echo Building QR Scanner...

if exist build rmdir /s /q build
mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% NEQ 0 (
    echo CMake configure error
    pause
    exit /b 1
)

cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo Build failed
    pause
    exit /b 1
)

cd ..
echo Build complete.

if exist build\Release\qr_detector.exe (
    build\Release\qr_detector.exe
) else (
    echo Executable not found
)
pause
