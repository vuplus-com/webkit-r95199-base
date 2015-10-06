#!/bin/sh
CONFIG_SHELL=/bin/bash
export CONFIG_SHELL
CC=sh4-linux-gcc
export CC
AS=sh4-linux-as
export AS
LD=sh4-linux-ld
export LD
OBJDUMP=sh4-linux-objdump
export OBJDUMP
AR=sh4-linux-ar
export AR
RANLIB=sh4-linux-ranlib
export RANLIB
CXX=sh4-linux-g++
export CXX
NM=sh4-linux-nm
export NM
STRIP=true
export STRIP
SED=sed
export SED
ARCH=sh4
'[' sh4 = st231 ']'
CFLAGS='-Wformat -Wformat-security -D_FORTIFY_SOURCE=2' 
CXXFLAGS='-Wformat -Wformat-security -D_FORTIFY_SOURCE=2'
export CFLAGS
export CXXFLAGS
LIBTOOL_PREFIX_BASE=/opt/STM/STLinux-2.4/devkit/sh4/target
export LIBTOOL_PREFIX_BASE
PKG_CONFIG_SYSROOT_DIR=/opt/STM/STLinux-2.4/devkit/sh4/target
export PKG_CONFIG_SYSROOT_DIR
PKG_CONFIG_LIBDIR=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig
export PKG_CONFIG_LIBDIR
unset PKG_CONFIG_PATH
FREETYPE_CONFIG=/opt/STM/STLinux-2.4/devkit/sh4/bin/sh4-linux-freetype-config
export FREETYPE_CONFIG
FT2_CONFIG=/opt/STM/STLinux-2.4/devkit/sh4/bin/sh4-linux-freetype-config
export FT2_CONFIG
SDL_CONFIG=/opt/STM/STLinux-2.4/devkit/sh4/bin/sh4-linux-sdl-config
export SDL_CONFIG
ACLOCAL_PATH=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/share/aclocal
export ACLOCAL_PATH
export LDFLAGS=-L/home/kdhong/webkit/ST/sysroot/usr/lib
CONFIG_SITE=/opt/STM/STLinux-2.4/config/config.site.sh4
export CONFIG_SITE
#rm -rf /home/kdhong/rpmbuild/BUILDROOT/stlinux24-target-webkit-1.6-17.i386
#true
#export CFLAGS=
#CFLAGS=
#export CXXFLAGS=
#CXXFLAGS=
#for p in libgtk-2.0-directfb libcairo-directfb
#pp=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libgtk-2.0-directfb
export PKG_CONFIG_LIBDIR=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libgtk-2.0-directfb:/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig
PKG_CONFIG_LIBDIR=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libgtk-2.0-directfb:/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig
#for p in libgtk-2.0-directfb libcairo-directfb
#pp=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libcairo-directfb
export PKG_CONFIG_LIBDIR=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libcairo-directfb:/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libgtk-2.0-directfb:/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig
PKG_CONFIG_LIBDIR=/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libcairo-directfb:/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig/libgtk-2.0-directfb:/opt/STM/STLinux-2.4/devkit/sh4/target/usr/lib/pkgconfig
export ac_cv_path_icu_config=/opt/STM/STLinux-2.4/devkit/sh4/bin/sh4-linux-icu-config
ac_cv_path_icu_config=/opt/STM/STLinux-2.4/devkit/sh4/bin/sh4-linux-icu-config
export GLIB_CFLAGS="-I/home/kdhong/webkit/ST/sysroot/usr/include/glib-2.0 -I/home/kdhong/webkit/ST/sysroot/usr/lib/glib-2.0/include"
export GLIB_LIBS='-L/home/kdhong/webkit/ST/sysroot/usr/lib -lglib-2.0'
export LIBSOUP_CFLAGS='-pthread -I/home/kdhong/webkit/ST/sysroot/usr/include/libsoup-2.4 -I/home/kdhong/webkit/ST/sysroot/usr/include/libxml2 -I/home/kdhong/webkit/ST/sysroot/usr/include/glib-2.0 -I/home/kdhong/webkit/ST/sysroot/usr/lib/glib-2.0/include'
export LIBSOUP_LIBS='-pthread -L/home/kdhong/webkit/ST/sysroot/usr/lib -lsoup-2.4 -lgio-2.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0'

GLIB_GENMARSHAL=/opt/STM/STLinux-2.4/host/bin/target-glib-genmarshal \
				./configure \
				--with-gtk=2.0 \
				--host=sh4-linux \
				--prefix=/usr \
				--exec-prefix=/usr \
				--bindir=/usr/bin \
				--sbindir=/usr/sbin \
				--sysconfdir=/etc \
				--datadir=/usr/share \
				--includedir=/usr/include \
				--libdir=/usr/lib \
				--libexecdir=/usr/libexec \
				--localstatedir=/var \
				--sharedstatedir=/usr/share \
				--mandir=/usr/share/man \
				--infodir=/usr/share/info \
				GLIB_COMPILE_SCHEMAS=/opt/STM/STLinux-2.4/host/bin/target-glib-compile-schemas \
				--disable-spellcheck \
				--enable-optimizations \
				--disable-channel-messaging \
				--disable-meter-tag \
				--disable-javascript-debugger \
				--enable-image-resizer \
				--disable-sandbox \
				--disable-xpath \
				--disable-xslt \
				--disable-svg \
				--disable-filters \
				--disable-svg-fonts \
				--disable-video \
				--disable-video-track \
				--without-x \
				--with-target=directfb \
				--disable-jit \
				--enable-fast-malloc \
				--enable-shared-workers \
				--enable-workers \
				--enable-fast-mobile-scrolling \
				--enable-offline-web-applications \


				




