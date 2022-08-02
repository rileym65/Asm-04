PROJECT=asm04
CC=gcc
DEFS=
INDIR=
INCS=
LIBDIR=
LIBS=
OBJS=\
	main.o

$(PROJECT): $(OBJS)
	$(CC) $(DEFS) $(LIBDIR) $(OBJS) $(LIBS) -o $(PROJECT)

.c.o:
	$(CC) $(DEFS) $(INCDIR) $(INCS) -c $<

clean:
	-rm *.o
	-rm $(PROJECT)

install:
	cp $(PROJECT) /usr/local/bin
	chmod a+x /usr/local/bin/$(PROJECT)

main.o:                header.h main.c

