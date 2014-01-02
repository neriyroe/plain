#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 01/02/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

all: clean development

clean:
	rm -rf libpfvl.dylib
	rm -rf pfvx86
	rm -rf Intermediate
	rm -rf Binary
	rm -rf Include
	rm -rf Library

development:
	if [ ! -d Intermediate ]; then \
	    mkdir Intermediate; \
	fi
	cd Intermediate && cmake ../../../ "-DCMAKE_OSX_ARCHITECTURES=i386;x86_64" -DMOCOSEL_DEBUGGING=1
	cd Intermediate && make
	cp Intermediate/Source/libpfvl.dylib libpfvl.dylib
	cp Intermediate/Utilities/pfvx86/pfvx86 pfvx86

distribution: clean
	mkdir Intermediate
	mkdir Binary
	mkdir Library
	cd Intermediate && cmake ../../../ "-DCMAKE_OSX_ARCHITECTURES=i386;x86_64"
	cd Intermediate && make
	cp Intermediate/Source/libpfvl.dylib Library/libpfv1.dylib
	cp Intermediate/Utilities/pfvx86/pfvx86 Binary/pfvx86
	cp -R ../../Include Include
