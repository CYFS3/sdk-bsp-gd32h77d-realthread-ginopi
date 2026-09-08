@echo off
cd /d "%~dp0"
if not exist rt-thread mklink /J rt-thread ..\..\rt-thread
if not exist libraries mklink /J libraries ..\..\libraries
if not exist packages mkdir packages
if not exist packages\LVGL-latest mklink /J packages\LVGL-latest ..\..\packages\LVGL-latest
if not exist packages\gt911-latest mklink /J packages\gt911-latest ..\..\packages\gt911-latest
