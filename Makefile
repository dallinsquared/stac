CC = gcc
EXES = stac getsizes

stac: sizedefs.h sta.c
	$(CC) sta.c -o stac

sizedefs.h: getsizes.c 
	$(CC) getsizes.c -o getsizes
	./getsizes > sizedefs.h

clean:
	rm -f $(EXES) sizedefs.h
