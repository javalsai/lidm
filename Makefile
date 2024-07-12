CDIR=src
LDIR=lib
IDIR=include
ODIR=dist

CC=gcc
CFLAGS=-I$(IDIR)

LIBS=-lm

_DEPS = util.h ui.h efield.h keys.h users.h sessions.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o util.o ui.o efield.o users.o sessions.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

lidm: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *- li $(INCDIR)/*-
