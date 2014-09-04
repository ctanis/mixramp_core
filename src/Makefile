LIB=libramp.a
SRC=mixramp_core.cpp mixramp_sdl.cpp mixramp_mixer.cpp audioFace.cpp

include ../make.skel

all: lib

clean: srcclean
	-rm -f $(LIB)


# for flymake syntax checking
.PHONY: check-syntax
check-syntax:
	$(CXX) -I../include -Wall -Wno-long-long -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
