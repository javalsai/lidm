VERSION = 1.2.0
.DEFAULT_GOAL := lidm

CDIR=src
LDIR=lib
IDIR=include
ODIR=dist

PREFIX=/usr

CC?=gcc
CFLAGS?=-O3 -Wall
ALLFLAGS=$(CFLAGS) -I$(IDIR)

LIBS=-lpam

_DEPS = version.h log.h util.h ui.h ui_state.h config.h desktop.h auth.h ofield.h efield.h keys.h users.h sessions.h chvt.h macros.h launch_state.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o log.o util.o ui.o ui_state.o config.o desktop.o auth.o ofield.o efield.o users.o sessions.o chvt.o launch_state.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.git/HEAD:

$(IDIR)/version.h: Makefile .git/HEAD
	@tmp=$$(mktemp); \
	printf '' > $$tmp; \
	echo '#define LIDM_VERSION "'$(VERSION)'"' >> $$tmp; \
	echo '#define LIDM_GIT_REV "'$$(git describe --long --tags --always || echo '?')'"' >> $$tmp; \
	echo '#define LIDM_BUILD_TS '$$(date +%s) >> $$tmp; \
	if ! cmp -s $$tmp $@; then \
		mv $$tmp $@; \
	fi; \
	rm -f $$tmp;

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(ALLFLAGS)

lidm: $(OBJ)
	$(CC) -o $@ $^ $(ALLFLAGS) $(LIBS)

clean:
	rm -f $(ODIR)/*.o lidm

# Copy lidm to ${DESTDIR}${PREFIX}/bin (/usr/bin)
install: lidm
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${PREFIX}/share/man/man{1,5}
	install -Dm755 ./lidm ${DESTDIR}${PREFIX}/bin/
	[ -f ${DESTDIR}/etc/lidm.ini ] || install -Dm644 ./themes/default.ini ${DESTDIR}/etc/lidm.ini
	install -Dm644 ./assets/man/lidm.1 ${DESTDIR}${PREFIX}/share/man/man1/
	install -Dm644 ./assets/man/lidm-config.5 ${DESTDIR}${PREFIX}/share/man/man5/

uninstall:
	rm -rf ${DESTDIR}${PREFIX}/bin/lidm ${DESTDIR}/etc/lidm.ini
	rm -rf ${DESTDIR}/usr/share/man/man{1/lidm.1,5/lidm-config.5}.gz
	rm -rf ${DESTDIR}/etc/systemd/system/lidm.service ${DESTDIR}/etc/dinit.d/lidm ${DESTDIR}/etc/runit/sv/lidm

install-service:
	@if command -v systemctl &> /dev/null; then \
		make install-service-systemd; \
	elif command -v dinitctl &> /dev/null; then \
		make install-service-dinit; \
	elif command -v sv &> /dev/null; then \
		if [ -d /etc/sv ]; then \
			make install-service-runit; \
		elif [ -d /etc/runit/sv ]; then \
			make install-service-runit-etc; \
		else \
			printf '\033[31m%s\033[0m\n' "Unknown init system structure, skipping service install..." >&2; \
		fi \
	elif command -v rc-update &> /dev/null; then \
		make install-service-openrc; \
	elif command -v s6-service &> /dev/null; then \
		if [ -d /etc/sv ]; then\
			make install-service-s6; \
		elif [ -d /etc/r6nit/sv ]; then \
			make install-service-s6-etc; \
		else \
			printf '\033[31m%s\033[0m\n' "Unknown init system structure, skipping service install..." >&2; \
		fi \
	else \
		printf '\033[1;31m%s\033[0m\n' "Unknown init system, skipping service install..." >&"; \
	fi

install-service-systemd:
	install -m644 ./assets/services/systemd.service ${DESTDIR}/etc/systemd/system/lidm.service
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'systemctl enable lidm'"
install-service-dinit:
	install -m644 ./assets/services/dinit ${DESTDIR}/etc/dinit.d/lidm
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'dinitctl enable lidm'"
install-service-runit:
	@if [ ! -e /etc/sv ] && [ -z "$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "${DESTDIR}/etc/sv doesn't exist" >&2 \
		exit 1 \
	fi
	mkdir -p ${DESTDIR}/etc/runit/lidm
	cp -r --update=all ./assets/services/runit/* ${DESTDIR}/etc/sv/lidm/
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'ln -s ${DESTDIR}/etc/sv/lidm /var/service'"
install-service-runit-etc:
	@if [ ! -e /etc/runit/sv ] && [ -z "$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "${DESTDIR}/etc/s6/sv doesn't exist" >&2 \
		exit 1 \
	fi
	mkdir -p ${DESTDIR}/etc/runit/sv/lidm
	cp -r --update=all ./assets/services/runit/* ${DESTDIR}/etc/s6/sv/lidm/
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'ln -s ${DESTDIR}/etc/s6/sv/lidm /var/service'"
install-service-openrc:
	install -m755 ./assets/services/openrc ${DESTDIR}/etc/init.d/lidm
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'rc-update add lidm'"
install-service-s6:
	@if [ ! -e "${DESTDIR}/etc/sv" ] && [ -z "$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "${DESTDIR}/etc/sv doesn't exist" >&2 \
		exit 1 \
	fi
	mkdir -p ${DESTDIR}/etc/sv/lidm
	cp -r --update=all ./assets/services/s6/* ${DESTDIR}/etc/sv/lidm/
install-service-s6-etc:
	@if [ ! -e "${DESTDIR}/etc/s6/sv" ] && [ -z "$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "${DESTDIR}/etc/s6/sv doesn't exist" >&2 \
		exit 1 \
	fi
	mkdir -p ${DESTDIR}/etc/s6/sv/lidm
	cp -r --update=all ./assets/services/s6/* ${DESTDIR}/etc/s6/sv/lidm/
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 's6-service add default lidm' and 's6-db-reload'"

pre-commit:
	codespell
	find . -type f -name '*.sh' -not -path './assets/pkg/aur/*/src/*' | xargs shellcheck
	clang-format -i $$(git ls-files "*.c" "*.h")
	clang-tidy -p . $$(git ls-files "*.c" "*.h")

print-version:
	@echo $(VERSION)

.PHONY: clean \
	install uninstall \
	install-service \
	install-service-s6 \
	install-service-s6-etc \
	install-service-dinit \
	install-service-runit \
	install-service-runit-etc \
	install-service-openrc \
	install-service-systemd \
	pre-commit \
	print-version
