VERSION = 1.2.3
.DEFAULT_GOAL := lidm

CDIR=src
LDIR=lib
IDIR=include
ODIR=dist

PREFIX=/usr/local

CC?=gcc
CFLAGS?=-O3 -Wall -Wextra -fdata-sections -ffunction-sections
# C PreProcessor flags, not C Plus Plus
CPPFLAGS?=
ALLFLAGS=$(CFLAGS) $(CPPFLAGS) -I$(IDIR)
LDFLAGS?=-Wl,--gc-sections

LIBS=-lpam

_DEPS = version.h log.h util.h ui.h ui_state.h config.h pam.h desktop.h desktop_exec.h auth.h ofield.h efield.h keys.h users.h sessions.h chvt.h macros.h launch_state.h signal_handler.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o log.o util.o ui.o ui_state.o config.o pam.o desktop.o desktop_exec.o auth.o ofield.o efield.o users.o sessions.o chvt.o launch_state.o signal_handler.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

INFO_GIT_REV?=$$(git describe --long --tags --always || echo '?')
INFO_BUILD_TS?=$$(date +%s)

$(IDIR)/version.h: Makefile
	@tmp=$$(mktemp); \
	printf '' > $$tmp; \
	echo '#define LIDM_VERSION "'$(VERSION)'"' >> $$tmp; \
	echo '#define LIDM_GIT_REV "'$(INFO_GIT_REV)'"' >> $$tmp; \
	echo '#define LIDM_BUILD_TS '$(INFO_BUILD_TS) >> $$tmp; \
	if ! cmp -s $$tmp $@; then \
		mv $$tmp $@; \
	fi; \
	rm -f $$tmp;

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(ALLFLAGS)

lidm: $(OBJ)
	$(CC) -o $@ $^ $(ALLFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(ODIR)/*.o lidm

install: lidm
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${PREFIX}/share/man/man{1,5}
	install -Dm755 ./lidm ${DESTDIR}${PREFIX}/bin/
	[ -f ${DESTDIR}/etc/lidm.ini ] || install -Dm644 ./themes/default.ini ${DESTDIR}/etc/lidm.ini
	install -Dm644 ./assets/man/lidm.1 ${DESTDIR}${PREFIX}/share/man/man1/
	install -Dm644 ./assets/man/lidm-config.5 ${DESTDIR}${PREFIX}/share/man/man5/

uninstall: uninstall-service
	rm -rf ${DESTDIR}${PREFIX}/bin/lidm ${DESTDIR}/etc/lidm.ini
	rm -rf ${DESTDIR}${PREFIX}/share/man/man{1/lidm.1,5/lidm-config.5}.gz

include services.mk

pre-commit:
	codespell
	prettier -c "**/*.md"
	git ls-files "*.sh" "*/PKGBUILD" | xargs shellcheck --shell=bash
	clang-format -i $$(git ls-files "*.c" "*.h")
	git ls-files -z "*.c" "*.h" | \
		parallel -j$$(nproc) -q0 --no-notice --will-cite --tty clang-tidy -warnings-as-errors=\* --quiet |& \
		grep -v "warnings generated." || true

print-version:
	@echo $(VERSION)

.PHONY: clean \
	install uninstall \
	pre-commit \
	print-version
