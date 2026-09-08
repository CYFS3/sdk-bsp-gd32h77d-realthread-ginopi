@echo off
cd /d "%~dp0"
if not exist rt-thread mklink /J rt-thread ..\..\rt-thread
if not exist libraries mklink /J libraries ..\..\libraries
if not exist packages mkdir packages
if not exist packages\at_device-latest mklink /J packages\at_device-latest ..\..\packages\at_device-latest
if not exist packages\kawaii-mqtt-latest mklink /J packages\kawaii-mqtt-latest ..\..\packages\kawaii-mqtt-latest
