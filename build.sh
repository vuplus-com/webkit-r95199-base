#!/bin/sh
./Tools/Scripts/build-webkit --gtk --debug \
				--with-gtk=2.0 \
				--enable-fast-mobile-scrolling \
				--with-font-backend=freetype \
				--no-mathml \
				--no-media-stream \
				--no-svg \
				--no-svg-fonts \
				--disable-spellcheck \
				--no-geolocation
			



