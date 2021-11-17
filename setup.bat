@echo off
SetLocal EnableDelayedExpansion

call msvc.bat

rmdir /s /q build
meson setup build --buildtype release || exit 1
@REM meson setup build --buildtype debug || exit 1
@REM meson setup build --buildtype debugoptimized || exit 1
@REM meson setup build --buildtype debug --unity on --unity-size 1000 || exit 1
