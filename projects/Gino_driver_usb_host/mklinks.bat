@echo off
cd /d "%~dp0"
if not exist rt-thread mklink /J rt-thread ..\..\rt-thread
if not exist libraries mklink /J libraries ..\..\libraries
