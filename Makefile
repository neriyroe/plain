INCLUDE	=	Include
SOURCE	=	Source/Plain/Shared/Algorithm/Hash.c \
			Source/Plain/Shared/Concat.c \
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

X86	    =	Source/x86/system/fetch.c \
			Source/x86/plain.c \

clean:
	rm -rf Binary/x86/* Binary/x86/*.*

x86:
	gcc -I $(INCLUDE) -o Binary/x86/plain -O3 $(SOURCE) $(X86) -D MOCOSEL_DEBUGGING
