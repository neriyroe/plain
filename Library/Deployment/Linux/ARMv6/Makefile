#
# Author   Neriy Roe <nr@mocosel.com>.
# Date     11/05/2014.
# Revision 09/01/2015.
#
# Copyright 2015 Nerijus Ramanauskas.
#

home = ../../..
target = Linux/ARMv6

.PHONY: all
all: clean development

.PHONY: clean
clean:
	rm -rf $(home)/$(target)
	rm -rf $(home)/Linux/ARM

.PHONY: development
development:
	if [ ! -d $(home)/$(target) ]; then \
		mkdir -p $(home)/$(target); \
	fi

	cd $(home)/$(target) && \
	cmake `dirname $(home)` -DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabi-gcc -DCMAKE_C_FLAGS="-march=armv6 -static" -DMOCOSEL_DEBUGGING=1 -DMOCOSEL_EMBEDDED=1 && \
	make

.PHONY: distribution
distribution: clean
	if [ ! -d $(home)/$(target) ]; then \
		mkdir -p $(home)/$(target); \
	fi

	cd $(home)/$(target) && \
	cmake `dirname $(home)` -DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabi-gcc -DCMAKE_C_FLAGS="-march=armv6 -static" -DMOCOSEL_EMBEDDED=1 && \
	make && \
	rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile
