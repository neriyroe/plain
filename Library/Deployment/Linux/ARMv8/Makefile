#
# Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
# Date     11/08/2014.
# Revision 09/01/2015.
#
# Copyright 2015 Nerijus Ramanauskas.
#

home = ../../..
target = Linux/ARMv8

.PHONY: all
all: clean development

.PHONY: clean
clean:
	rm -rf $(home)/$(target)

.PHONY: development
development:
	if [ ! -d $(home)/$(target) ]; then \
		mkdir -p $(home)/$(target); \
	fi

	cd $(home)/$(target) && \
	cmake `dirname $(home)` -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc -DCMAKE_C_FLAGS=-static -DPLAIN_DEBUGGING=1 -DPLAIN_EMBEDDED=1 && \
	make

.PHONY: distribution
distribution: clean
	if [ ! -d $(home)/$(target) ]; then \
		mkdir -p $(home)/$(target); \
	fi

	cd $(home)/$(target) && \
	cmake `dirname $(home)` -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc -DCMAKE_C_FLAGS=-static -DPLAIN_EMBEDDED=1 && \
	make && \
	rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile
