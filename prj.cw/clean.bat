@echo off
echo Cleaning build directory...
if exist "build" (
    rmdir /s /q build
    echo Build directory cleaned
) else (
    echo Build directory does not exist
)
pause
