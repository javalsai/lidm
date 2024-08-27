CDIR=src
LDIR=lib
IDIR=include
ODIR=dist

PREFIX=/usr

CC?=gcc
CFLAGS?=-O3
_CFLAGS=-I$(DIR)
ALLFLAGS=$(CFLAGS) -I$(IDIR)

LIBS=-lm -lpam -lpam_misc

_DEPS = util.h ui.h config.h auth.h efield.h keys.h users.h sessions.h chvt.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o util.o ui.o config.o auth.o efield.o users.o sessions.o chvt.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(ALLFLAGS)

lidm: $(OBJ)
	$(CC) -o $@ $^ $(ALLFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *- li $(INCDIR)/*-

# Copy lidm to ${DESTDIR}${PREFIX}/bin (/usr/bin)
install: lidm
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}/etc
	install -m755 ./lidm ${DESTDIR}${PREFIX}/bin
	install -m755 ./themes/default.ini ${DESTDIR}/etc/lidm.ini

uninstall:
	rm -rf ${DESTDIR}${PREFIX}/bin/lidm ${DESTDIR}/etc/lidm.ini

install-service:
	@if command -v systemctl &> /dev/null; then \
		make install-service-systemd; \
	elif command -v dinitctl &> /dev/null; then \
		make install-service-dinit; \
	else \
		printf '\x1b[1;31m%s\x1b[0m\n' "Unknown init system, skipping install..."; \
	fi

install-service-systemd:
	install -m755 ./assets/services/systemd.service /etc/systemd/system/lidm.service
	@printf '\x1b[1m%s\x1b[0m\n\n' " don't forget to run 'systemctl enable lidm'"
install-service-dinit:
	install -m755 ./assets/services/dinit /etc/dinit.d/lidm
	@printf '\x1b[1m%s\x1b[0m\n\n' " don't forget to run 'dinitctl enable lidm'"
