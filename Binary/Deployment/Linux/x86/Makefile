#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     11/05/2014,
# Revision 11/05/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
#

home = ../../..
target = Linux/x86

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
	cmake `dirname $(home)` -DCMAKE_C_FLAGS=-m32 -DMOCOSEL_DEBUGGING=1 && \
	make
