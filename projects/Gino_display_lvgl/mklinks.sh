#!/bin/sh
[ -e rt-thread ] || ln -s ../../rt-thread rt-thread
[ -e libraries ] || ln -s ../../libraries libraries
[ -d packages ] || mkdir packages
[ -e packages/LVGL-latest ] || ln -s ../../../packages/LVGL-latest packages/LVGL-latest
[ -e packages/gt911-latest ] || ln -s ../../../packages/gt911-latest packages/gt911-latest
