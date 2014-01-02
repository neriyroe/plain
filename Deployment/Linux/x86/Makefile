#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 01/02/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

all: clean development

clean:
	rm -rf libpfvl.so
	rm -rf pfvx86
	rm -rf Intermediate
	rm -rf Binary
	rm -rf Include
	rm -rf Library

development:
	if [ ! -d Intermediate ]; then \
	    mkdir Intermediate; \
	fi
	cd Intermediate && cmake ../../../../ -DCMAKE_C_FLAGS=-m32 -DMOCOSEL_DEBUGGING=1
	cd Intermediate && make
	cp Intermediate/Source/libpfvl.so libpfvl.so
	cp Intermediate/Utilities/pfvx86/pfvx86 pfvx86

distribution: clean
	mkdir Intermediate
	mkdir Binary
	mkdir Library
	cd Intermediate && cmake ../../../../ -DCMAKE_C_FLAGS=-m32
	cd Intermediate && make
	cp Intermediate/Source/libpfvl.so Library/libpfv1.so
	cp Intermediate/Utilities/pfvx86/pfvx86 Binary/pfvx86
	cp -R ../../../Include Include
