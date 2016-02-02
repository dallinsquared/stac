CC = gcc

stac: sizedefs.h sta.c
	CC sta.c -o stac

sizedefs.h: getsizes.c sizeinclude
	CC getsizes.c -o getsizes
	./getsizes | cat sizeinclude - > sizedefs.h

