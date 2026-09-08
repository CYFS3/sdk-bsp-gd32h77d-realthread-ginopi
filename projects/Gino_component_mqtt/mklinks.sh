#!/bin/sh
[ -e rt-thread ] || ln -s ../../rt-thread rt-thread
[ -e libraries ] || ln -s ../../libraries libraries
[ -e packages/at_device-latest ] || ln -s ../../../packages/at_device-latest packages/at_device-latest
[ -e packages/kawaii-mqtt-latest ] || ln -s ../../../packages/kawaii-mqtt-latest packages/kawaii-mqtt-latest
