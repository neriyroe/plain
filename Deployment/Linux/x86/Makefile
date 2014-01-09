#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 01/09/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

all: clean development

clean:
	rm -rf pfvx86
	rm -rf Intermediate
	rm -rf Include
	rm -rf Library

development:
	if [ ! -d Intermediate ]; then \
	    mkdir Intermediate; \
	fi
	cd Intermediate && cmake ../../../../ -DCMAKE_C_FLAGS=-m32 -DMOCOSEL_DEBUGGING=1
	cd Intermediate && make
	cp Intermediate/Development/pfvx86/pfvx86 pfvx86

distribution: clean
	mkdir Intermediate
	mkdir Library
	cd Intermediate && cmake ../../../../ -DCMAKE_C_FLAGS=-m32
	cd Intermediate && make
	cp Intermediate/Source/libpfvl.so Library/libpfv1.so
	cp -R ../../../Include Include
