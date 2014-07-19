#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     07/20/2014,
# Revision 07/20/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
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
