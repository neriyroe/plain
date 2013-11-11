INCLUDE	= 	Include
SOURCE	=	Source/Plain/Concat.c \
	        Source/Plain/Hash.c \
	        Source/Plain/Join.c \
	        Source/Plain/Lookup.c \
	        Source/Plain/Purge.c \
	        Source/Plain/Register.c \
	        Source/Plain/x86/Resize.c \
		    Source/Plain/Tokenize.c \
	        Source/Plain/Unregister.c \
	        Source/Plain/Walk.c \
	        Source/Plain/Yield.c \
	        Source/Plain/Framework/Finalize.c \
		    Source/Plain/Framework/Run.c \
	        Source/Plain/Framework/Version.c
X86	    =	Source/x86/system/fetch.c \
	        Source/x86/plain.c

clean:
	rm -rf Binary/x86/* Binary/x86/*.*

x86:
	gcc -I $(INCLUDE) -o Binary/x86/plain -O3 $(SOURCE) $(X86)
