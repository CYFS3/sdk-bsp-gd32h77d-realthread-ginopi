#!/bin/sh
[ -e rt-thread ] || ln -s ../../rt-thread rt-thread
[ -e libraries ] || ln -s ../../libraries libraries
[ -d packages ] || mkdir packages
[ -e packages/i2c-tools-v1.0.0 ] || ln -s ../../../packages/i2c-tools-v1.0.0 packages/i2c-tools-v1.0.0
