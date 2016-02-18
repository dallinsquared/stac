CCOPTS = -std=c99 -pedantic
CC = gcc $(CCOPTS)
EXES = stac getsizes primex
GENS = primdefs.h sizedefs.h

stac: sizedefs.h primdefs.h sta.c
	$(CC) sta.c -o stac

debug: sizedefs.h sta.c
	$(CC)  sta.c -o stac -ggdb

primdefs.h: primex.c genprim.sh
	$(CC) primex.c -o primex
	./genprim.sh <sta.c | ./primex >primdefs.h

sizedefs.h: getsizes.c 
	$(CC) getsizes.c -o getsizes
	./getsizes > sizedefs.h

clean:
	rm -rfv $(EXES) $(GENS)
