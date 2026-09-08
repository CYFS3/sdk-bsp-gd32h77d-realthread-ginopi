#!/bin/sh
[ -e rt-thread ] || ln -s ../../rt-thread rt-thread
[ -e libraries ] || ln -s ../../libraries libraries
[ -e packages/LVGL-latest ] || ln -s ../../../packages/LVGL-latest packages/LVGL-latest
[ -e packages/gt911-latest ] || ln -s ../../../packages/gt911-latest packages/gt911-latest
[ -e packages/at_device-latest ] || ln -s ../../../packages/at_device-latest packages/at_device-latest
[ -e packages/kawaii-mqtt-latest ] || ln -s ../../../packages/kawaii-mqtt-latest packages/kawaii-mqtt-latest
[ -e packages/i2c-tools-v1.0.0 ] || ln -s ../../../packages/i2c-tools-v1.0.0 packages/i2c-tools-v1.0.0
