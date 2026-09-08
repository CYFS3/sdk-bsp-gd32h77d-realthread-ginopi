@echo off
cd /d "%~dp0"
if not exist rt-thread mklink /J rt-thread ..\..\rt-thread
if not exist libraries mklink /J libraries ..\..\libraries
if not exist packages mkdir packages
if not exist packages\i2c-tools-v1.0.0 mklink /J packages\i2c-tools-v1.0.0 ..\..\packages\i2c-tools-v1.0.0
