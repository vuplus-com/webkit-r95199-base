#!/bin/sh

#./WebKitBuild/Release/Programs/GtkLauncher --enable-developer-extras=true --enable-spatial-navigation=true http://192.168.3.222/tmp/none.html > pipe 2>&1 &
./WebKitBuild/Debug/Programs/GtkLauncher --enable-developer-extras=true --enable-spatial-navigation=true file:///home/kdhong/webkit/ST/x86/Webkit-r95199-x86.release/none.html > pipe 2>&1 &
