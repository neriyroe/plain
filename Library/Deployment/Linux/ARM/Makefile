#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     06/22/2014,
# Revision 06/22/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
#

home = ../../..
target = Linux/ARM

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
	cmake `dirname $(home)` -DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabi-gcc -DCMAKE_C_FLAGS="-march=armv6" -DMOCOSEL_DEBUGGING=1 && \
	make
