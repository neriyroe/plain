#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     01/03/2014,
# Revision 02/15/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
#

clean:
	cd Deployment/Darwin/x86 && make clean
	cd Deployment/Linux/x86 && make clean
	find . -name .DS_Store -exec rm -rf -- {} \;
	find . -name Thumbs.db -exec rm -rf -- {} \;
