#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     06/01/2014,
# Revision 06/01/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
#

all: clean development

clean:
	rm -rf Include
	rm -rf Intermediate
	rm -rf Library

development:
	if [ ! -d Intermediate ]; then \
	    mkdir Intermediate; \
	fi
	cd Intermediate && cmake ../../../../ -DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabi-gcc -DCMAKE_C_FLAGS="-march=armv6" -DMOCOSEL_DEBUGGING=1 -DMOCOSEL_EMBEDDED=1
	cd Intermediate && make

distribution: clean
	mkdir Intermediate
	mkdir Library
	cd Intermediate && cmake ../../../../ -DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabi-gcc -DCMAKE_C_FLAGS="-march=armv6" -DMOCOSEL_EMBEDDED=1
	cd Intermediate && make
	cp Intermediate/Source/libplain.a Library/libplain.a
	cp -R ../../../Include Include
