CCOPTS = -std=c99 -pedantic
CC = gcc $(CCOPTS)
EXES = stac getsizes primex
GENS = primdefs.h sizedefs.h

stac: sizedefs.h primdefs.h sta.c
	$(CC) sta.c -o stac

debug: sizedefs.h sta.c
	$(CC)  sta.c -o stac -ggdb

primdefs.h: primex.c primitives
	$(CC) primex.c -o primex
	./primex <primitives >primdefs.h

sizedefs.h: getsizes.c 
	$(CC) getsizes.c -o getsizes
	./getsizes > sizedefs.h

clean:
	rm -rfv $(EXES) $(GENS)
