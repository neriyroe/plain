#
# Author   Neriy Roe <nr@mocosel.com>.
# Date     06/22/2014.
# Revision 09/01/2015.
#
# Copyright 2015 Nerijus Ramanauskas.
#

home = ../../..
target = Linux/x64

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
	cmake `dirname $(home)` -DCMAKE_C_FLAGS=-m64 -DMOCOSEL_DEBUGGING=1 && \
	make

.PHONY: distribution
distribution: clean
	if [ ! -d $(home)/$(target) ]; then \
		mkdir -p $(home)/$(target); \
	fi

	cd $(home)/$(target) && \
	cmake `dirname $(home)` -DCMAKE_C_FLAGS=-m64 && \
	make && \
	rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile
