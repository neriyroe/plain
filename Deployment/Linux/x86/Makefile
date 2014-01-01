#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 12/31/2013,
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
	cd Intermediate && cmake ../../../../ -DMOCOSEL_DEBUGGING=1 -DCMAKE_TOOLCHAIN_FILE=Deployment/Linux/x86/Target.cmake
	cd Intermediate && make
	cp Intermediate/Source/libpfvl.so libpfvl.so
	cp Intermediate/Utilities/pfvx86/pfvx86 pfvx86

distribution: clean
	mkdir Intermediate
	mkdir Binary
	mkdir Library
	cd Intermediate && cmake ../../../../
	cd Intermediate && make
	cp Intermediate/Source/libpfvl.so Library/libpfv1.so
	cp Intermediate/Utilities/pfvx86/pfvx86 Binary/pfvx86
	cp -R ../../../Include Include
