#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     01/03/2013,
# Revision 01/09/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

clean:
	cd Deployment/Darwin/x86 && make clean
	cd Deployment/Linux/x86 && make clean
	find . -name .DS_Store -exec rm -rf -- {} \;
	find . -name Thumbs.db -exec rm -rf -- {} \;
