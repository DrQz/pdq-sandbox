# Test PDQ7/lib C source changes
#
# Created by NJG on Saturday, May 21, 2016 - Set lib to GitHub path
# Updated by NJG on Tuesday, May 24, 2016  - Changed CFLAGS to remove 'linker' warnings
# Updated by NJG on Monday, December 11, 2017 
#   - mmmn-fesc and mmmn-prob models as self-contained codes
#   - Intend to incorporate into PDQ_Lib.h
   

CC     = gcc
#LIB    = /Users/njg/GitHub/PDQ/pdq-qnm-pkg/lib
LIB    = lib
#INCL   = /Users/njg/GitHub/PDQ/pdq-qnm-pkg/lib
INCL   = lib

#CFLAGS = -I $(INCL) -l $(LIB) -ggdb
#CFLAGS = -I $(INCL)  

PGMS	= \
		mmmn-fesc \
		mmmn-prob \
		mmmn-func

all: $(PGMS)

mmmn-fesc: mmmn-fesc.c mmmn-fesc.o
	${CC} -o mmmn-fesc mmmn-fesc.c -I$(INCL) -L$(LIB) -lpdq -lm

mmmn-prob: mmmn-prob.c mmmn-prob.o
	${CC} -o mmmn-prob mmmn-prob.c -I$(INCL) -L$(LIB) -lpdq -lm
	
mmmn-func: mmmn-func.c mmmn-func.o
	${CC} -o mmmn-func mmmn-func.c -I$(INCL) -L$(LIB) -lpdq -lm	


clean:
	-/bin/rm -f ${PGMS} *.o
