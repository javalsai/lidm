CDIR=src
LDIR=lib
IDIR=include
ODIR=dist

PREFIX=/usr

CC?=gcc
CFLAGS?=-O3 -Wall
_CFLAGS=-I$(DIR)
ALLFLAGS=$(CFLAGS) -I$(IDIR)

LIBS=-lpam

_DEPS = log.h util.h ui.h ui_state.h config.h desktop.h auth.h ofield.h efield.h keys.h users.h sessions.h chvt.h macros.h launch_state.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o log.o util.o ui.o ui_state.o config.o desktop.o auth.o ofield.o efield.o users.o sessions.o chvt.o launch_state.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(ALLFLAGS)

lidm: $(OBJ)
	$(CC) -o $@ $^ $(ALLFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o lidm

# Copy lidm to ${DESTDIR}${PREFIX}/bin (/usr/bin)
install: lidm
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${PREFIX}/share/man/man{1,5}
	install -Dm755 ./lidm ${DESTDIR}${PREFIX}/bin/
	[ ! -f ${DESTDIR}/etc/lidm.ini ] && install -Dm644 ./themes/default.ini ${DESTDIR}/etc/lidm.ini || true
	install -Dm644 ./assets/man/lidm.1 ${DESTDIR}${PREFIX}/share/man/man1/
	install -Dm644 ./assets/man/lidm-config.5 ${DESTDIR}${PREFIX}/share/man/man5/

uninstall:
	rm -rf ${DESTDIR}${PREFIX}/bin/lidm ${DESTDIR}/etc/lidm.ini
	rm -rf ${DESTDIR}/usr/share/man/man{1/lidm.1,5/lidm-config.5}.gz
	rm -rf /etc/systemd/system/lidm.service /etc/dinit.d/lidm /etc/runit/sv/lidm

install-service:
	@if command -v systemctl &> /dev/null; then \
		make install-service-systemd; \
	elif command -v dinitctl &> /dev/null; then \
		make install-service-dinit; \
	elif command -v sv &> /dev/null; then \
		make install-service-runit; \
	elif command -v rc-update &> /dev/null; then \
		make install-service-openrc; \
	elif command -v s6-service &> /dev/null; then \
		make install-service-s6; \
	else \
		printf '\x1b[1;31m%s\x1b[0m\n' "Unknown init system, skipping service install..."; \
	fi

install-service-systemd:
	install -m644 ./assets/services/systemd.service /etc/systemd/system/lidm.service
	@printf '\x1b[1m%s\x1b[0m\n\n' " don't forget to run 'systemctl enable lidm'"
install-service-dinit:
	install -m644 ./assets/services/dinit /etc/dinit.d/lidm
	@printf '\x1b[1m%s\x1b[0m\n\n' " don't forget to run 'dinitctl enable lidm'"
install-service-runit:
	rsync -a --no-owner --no-group ./assets/services/runit/. /etc/runit/sv/lidm
	@printf '\x1b[1m%s\x1b[0m\n\n' " don't forget to run 'ln -s /etc/runit/sv/lidm /run/runit/service' and 'sv enable lidm'"
install-service-openrc:
	install -m755 ./assets/services/openrc /etc/init.d/lidm
	@printf '\x1b[1m%s\x1b[0m\n\n' " don't forget to run 'rc-update add lidm'"
install-service-s6:
	rsync -a --no-owner --no-group ./assets/services/s6/. /etc/s6/sv/lidm
	@printf '\x1b[1m%s\x1b[0m\n\n' " don't forget to run 's6-service add default lidm' and 's6-db-reload'"

pre-commit:
	codespell
	find . -type f -name '*.sh' -not -path './assets/pkg/aur/*/src/*' | xargs shellcheck
	clang-format -i $$(git ls-files "*.c" "*.h")
	clang-tidy -p . $$(git ls-files "*.c" "*.h")
