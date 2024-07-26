CDIR=src
LDIR=lib
IDIR=include
ODIR=dist

CC=gcc
CFLAGS=-O3 -I$(IDIR)

LIBS=-lm -lpam -lpam_misc

_DEPS = util.h ui.h config.h auth.h efield.h keys.h users.h sessions.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o util.o ui.o config.o auth.o efield.o users.o sessions.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

lidm: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *- li $(INCDIR)/*-

# Copy lidm to /usr/bin
install: lidm
	install -m 755 lidm /usr/bin
	if command -v systemctl &> /dev/null; then \
		echo "Systemd exists, copying service file"; \
		cp assets/li.service /etc/systemd/system/; \
	else \
		echo "No systemd"; \
	fi
