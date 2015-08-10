#!/bin/sh

autoreconf --verbose --install --force  -I Source/autotools

export OE_TOP="/media/Extend/works/anga_oe"
export OE_CORE="$OE_TOP/openembedded-core"

export OE_SYSROOT="$OE_TOP/build/tmp/sysroots"
export LOCAL_SYSROOT="$OE_SYSROOT/i686-linux"
export TARGET_SYSROOT="$OE_SYSROOT/vusolo4k"

export TOOLCHAIN="$LOCAL_SYSROOT/usr/bin/arm-oe-linux-gnueabi"
export CROSS="$TOOLCHAIN/arm-oe-linux-gnueabi-"

export SYSROOT_EXTEND="--sysroot=$TARGET_SYSROOT "
export FPU_EXTEND="-march=armv7-a -mfloat-abi=hard -mfpu=neon "

export prefix="/usr"
export STRIP="arm-oe-linux-gnueabi-strip"
export localstatedir="/var"
export USER="oskwon"
export libexecdir="/usr/lib/webkit-gtk"
export datadir="/usr/share"
export BUILD_CXX="g++ "
export LD="arm-oe-linux-gnueabi-ld $SYSROOT_EXTEND "
export LDFLAGS="-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -Wl,--no-keep-memory"
export bindir="/usr/bin"
export TARGET_CXXFLAGS=" -Os -pipe -g -feliminate-unused-debug-types"
export includedir="/usr/include"
export BUILD_CC="gcc "
export BUILD_LDFLAGS="-L$LOCAL_SYSROOT/usr/lib -L$LOCAL_SYSROOT/lib -Wl,-rpath-link,$LOCAL_SYSROOT/usr/lib -Wl,-rpath-link,$LOCAL_SYSROOT/lib -Wl,-rpath,$LOCAL_SYSROOT/usr/lib -Wl,-rpath,$LOCAL_SYSROOT/lib -Wl,-O1"
export CPP_FOR_BUILD="gcc  -E"
unset TARGET_ARCH
export STRINGS="arm-oe-linux-gnueabi-strings"
export CCACHE_DIR="/home/oskwon"
export BUILD_LD="ld "
export oldincludedir="/usr/include"
export BUILD_CCLD="gcc "
export CFLAGS_FOR_BUILD="-isystem$LOCAL_SYSROOT/usr/include -O2 -pipe"
export BUILD_CFLAGS="-isystem$LOCAL_SYSROOT/usr/include -O2 -pipe"
export docdir="/usr/share/doc"
export infodir="/usr/share/info"
export CC="arm-oe-linux-gnueabi-gcc  $FPU_EXTEND $SYSROOT_EXTEND"
export TERM="xterm"
export RANLIB="arm-oe-linux-gnueabi-ranlib"
export CPPFLAGS=""
export base_sbindir="/sbin"
export CXX="arm-oe-linux-gnueabi-g++  $FPU_EXTEND $SYSROOT_EXTEND"
export systemd_unitdir="/lib/systemd"
export FC="arm-oe-linux-gnueabi-gfortran  $FPU_EXTEND $SYSROOT_EXTEND"
export HOME="/home/oskwon"
export BUILD_RANLIB="ranlib"
export LDFLAGS_FOR_BUILD="-L$LOCAL_SYSROOT/usr/lib -L$LOCAL_SYSROOT/lib -Wl,-rpath-link,$LOCAL_SYSROOT/usr/lib -Wl,-rpath-link,$LOCAL_SYSROOT/lib -Wl,-rpath,$LOCAL_SYSROOT/usr/lib -Wl,-rpath,$LOCAL_SYSROOT/lib -Wl,-O1"
export BUILD_FC="gfortran "
export BUILD_NM="nm"
export LD_FOR_BUILD="ld "
export lt_cv_sys_lib_dlsearch_path_spec="/usr/lib /lib"
export AS="arm-oe-linux-gnueabi-as "
export BUILD_CPPFLAGS="-isystem$LOCAL_SYSROOT/usr/include"
export CPP="arm-oe-linux-gnueabi-gcc -E $SYSROOT_EXTEND  $FPU_EXTEND"
export mandir="/usr/share/man"
export PKG_CONFIG_SYSROOT_DIR="$TARGET_SYSROOT"
export CONFIG_SITE="$OE_TOP/meta-openembedded/meta-oe/site/endian-little $OE_CORE/meta/site/endian-little $OE_CORE/meta/site/arm-common $OE_CORE/meta/site/arm-32 $OE_CORE/meta/site/common-linux $OE_CORE/meta/site/common-glibc $OE_CORE/meta/site/arm-linux $OE_CORE/meta/site/common $TARGET_SYSROOT/usr/share/arm-oe-linux-gnueabi_config_site.d/ncurses_config $TARGET_SYSROOT/usr/share/arm-oe-linux-gnueabi_config_site.d/glibc_config "
export BUILD_CXXFLAGS="-isystem$LOCAL_SYSROOT/usr/include -O2 -pipe"
export OBJCOPY="arm-oe-linux-gnueabi-objcopy"
export base_libdir="/lib"
export CCACHE_DISABLE="1"
export servicedir="/srv"
export PKG_CONFIG_PATH="$TARGET_SYSROOT/usr/lib/pkgconfig:$TARGET_SYSROOT/usr/share/pkgconfig"
export BUILD_AR="ar"
export LC_ALL="C"
export TARGET_CPPFLAGS=""
export PKG_CONFIG_DIR="$TARGET_SYSROOT/usr/lib/pkgconfig"
export sysconfdir="/etc"
export CCLD="arm-oe-linux-gnueabi-gcc  $FPU_EXTEND $SYSROOT_EXTEND"
export PATH="$OE_CORE/scripts:$LOCAL_SYSROOT/usr/bin/arm-oe-linux-gnueabi:$TARGET_SYSROOT/usr/bin/crossscripts:$LOCAL_SYSROOT/usr/sbin:$LOCAL_SYSROOT/usr/bin:$LOCAL_SYSROOT/sbin:$LOCAL_SYSROOT/bin:$OE_CORE/scripts:$OE_TOP/bitbake/bin:/usr/lib/lightdm/lightdm:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/opt/sublime_text_2"
export TARGET_CFLAGS=" -Os -pipe -g -feliminate-unused-debug-types"
export base_bindir="/bin"
export PKG_CONFIG_LIBDIR="$TARGET_SYSROOT/usr/lib/pkgconfig"
unset MACHINE
export sbindir="/usr/sbin"
export CFLAGS=" -Os -pipe -g -feliminate-unused-debug-types"
export BUILD_AS="as "
export CXXFLAGS_FOR_BUILD="-isystem$LOCAL_SYSROOT/usr/include -O2 -pipe"
export sharedstatedir="/com"
export OBJDUMP="arm-oe-linux-gnueabi-objdump"
unset DISTRO
export exec_prefix="/usr"
export TARGET_LDFLAGS="-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed"
export PKG_CONFIG_DISABLE_UNINSTALLED="yes"
export libdir="/usr/lib"
export nonarch_base_libdir="/lib"
export PSEUDO_DISABLED="1"
export CPPFLAGS_FOR_BUILD="-isystem$LOCAL_SYSROOT/usr/include"
export PSEUDO_UNLOAD="1"
export CC_FOR_BUILD="gcc "
export SHELL="/bin/bash"
export MAKE="make"
export CXX_FOR_BUILD="g++ "
export AR="arm-oe-linux-gnueabi-ar"
export BUILD_CPP="gcc  -E"
export PATCH_GET="0"
export CXXFLAGS=" -Os -pipe -g -feliminate-unused-debug-types -fvisibility-inlines-hidden"
export NM="arm-oe-linux-gnueabi-nm"
export LOGNAME="oskwon"
export base_prefix=""
export BUILD_STRIP="strip"

export PKG_CONFIG_LIBDIR="$TARGET_SYSROOT/usr/lib/pkgconfig"
export ac_cv_path_icu_config="$TARGET_SYSROOT/usr/bin/crossscripts/icu-config"
export GLIB_GENMARSHAL="$OE_SYSROOT/686-linux/usr/bin/glib-genmarshal"

export PATH="$TOOLCHAIN:$PATH"

GLIB_GENMARSHAL=$GLIB_GENMARSHAL ./configure \
	--build=i686-pc-linux-gnu \
	--host=arm-oe-linux-gnueabi \
	--with-sysroot=$TARGET_SYSROOT \
	--includedir=$TARGET_SYSROOT/usr \
	--oldincludedir=$TARGET_SYSROOT/usr/include \
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
	--enable-debug=no \
	--with-gtk=2.0 \
	--disable-spellcheck \
	--enable-optimizations \
	--disable-channel-messaging \
	--disable-meter-tag \
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
	--enable-javascript-debugger \
	--enable-fast-mobile-scrolling \
	--enable-offline-web-applications && \
sed -e "s/UNICODE_CFLAGS\ \=\ \-I\/usr\/include/UNICODE_CFLAGS\ \=\ \-I\/media\/Extend\/works\/anga_oe\/build\/tmp\/sysroots\/vusolo4k\/usr\/include/g" GNUmakefile > GNUmakefile2 \
	&& mv GNUmakefile2 GNUmakefile

