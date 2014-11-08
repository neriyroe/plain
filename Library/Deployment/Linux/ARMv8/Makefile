#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     11/08/2014,
# Revision 11/08/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
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
	cmake `dirname $(home)` -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc -DMOCOSEL_DEBUGGING=1 -DMOCOSEL_EMBEDDED=1 && \
	make

.PHONY: distribution
distribution: clean
	if [ ! -d $(home)/$(target) ]; then \
		mkdir -p $(home)/$(target); \
	fi

	cd $(home)/$(target) && \
	cmake `dirname $(home)` -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc -DMOCOSEL_EMBEDDED=1 && \
	make && \
	rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile
