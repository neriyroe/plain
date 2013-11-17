#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     11/13/2013,
# Revision 11/17/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

INCLUDE	=	Include
COMPACT	=	Source/Plain/Shared/Algorithm/Hash.c \
			Source/Plain/Shared/Concat.c \
			Source/Plain/Shared/Reserve.c \
			Source/Plain/Shared/Processing/Join.c \
			Source/Plain/Shared/Processing/Purge.c \
			Source/Plain/Shared/Processing/Tokenize.c \
			Source/Plain/x86/Resize.c \
			Source/Plain/Runtime/Lookup.c \
			Source/Plain/Runtime/Register.c \
			Source/Plain/Runtime/Unregister.c \
			Source/Plain/Runtime/Walk.c \
			Source/Plain/Framework/Run.c \
			Source/Plain/Framework/Version.c \
 			Source/Plain/Framework/Finalize.c \
 			Source/Plain/Auxiliary.c \
 			Source/Plain/Compact.c \

COMPLAIN  =	Source/Complain/x86/Session.c \
 			Source/Complain/x86/System/Load.c \
 			Source/Complain/x86/Complain.c \

x86:
	gcc -std=c99 -I $(INCLUDE) -o Binary/x86/Complain -O3 $(COMPACT) $(COMPLAIN) -D MOCOSEL_DEBUGGING -Wall

clean:
	rm -rf Binary/x86/* Binary/x86/*.*
	find . -name *.DS_Store -exec rm -rf {} \;
	find . -name *.DS_Store.* -exec rm -rf {} \;
