CDIR=src
LDIR=lib
IDIR=include
ODIR=dist

CC=gcc
CFLAGS=-I$(IDIR)

LIBS=-lm

_DEPS = util.h users.h sessions.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o util.o users.o sessions.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

li: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *- li $(INCDIR)/*-
