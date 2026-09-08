#!/bin/sh
[ -e rt-thread ] || ln -s ../../rt-thread rt-thread
[ -e libraries ] || ln -s ../../libraries libraries
[ -e packages/LVGL-latest ] || ln -s ../../../packages/LVGL-latest packages/LVGL-latest
[ -e packages/gt911-latest ] || ln -s ../../../packages/gt911-latest packages/gt911-latest
