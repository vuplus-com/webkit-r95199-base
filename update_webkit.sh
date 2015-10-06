#!/bin/sh

sh4-linux-strip .libs/lib*.so*
cp -a .libs/lib*.so* ~/devel/7105/STFAE/update/ufs924/webkit/usr/lib
cp Programs/GtkLauncher ~/devel/7105/STFAE/update/ufs924/webkit/bin/webkit_hbbtv
pushd .
cd ~/devel/7105/STFAE/update/ufs924/
rm webkit.cramfs
/sbin/mksquashfs webkit webkit.cramfs
cp webkit.cramfs ~/devel/7105/STFAE/Application.plugin
popd

