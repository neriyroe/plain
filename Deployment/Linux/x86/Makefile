#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 01/12/2014,
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
	cd Intermediate && cmake ../../../../ -DCMAKE_C_FLAGS=-m32 -DMOCOSEL_DEBUGGING=1
	cd Intermediate && make

distribution: clean
	mkdir Intermediate
	mkdir Library
	cd Intermediate && cmake ../../../../ -DCMAKE_C_FLAGS=-m32
	cd Intermediate && make
	cp Intermediate/Source/libplain.so Library/libplain.so
	cp -R ../../../Include Include
