all: nbndD12

nbndD12: nbndD12.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic nbndD12.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

nbndD12.o: nbndD12.c smpl.h
	$(COMPILE.c) -g  nbndD12.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o nbndD12 relat saida

