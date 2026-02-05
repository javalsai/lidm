VERSION = 2.0.0
.DEFAULT_GOAL := lidm

CDIR = src
LDIR = lib
IDIR = include
ODIR = dist

PREFIX = /usr/local

INFO_GIT_REV ?= $(shell git describe --long --tags --always || echo '?')
INFO_BUILD_TS ?= $(shell date +%s)

CFLAGS ?= -O3 -Wall -Wextra -fdata-sections -ffunction-sections
# C PreProcessor flags, not C Plus Plus
CPPFLAGS ?=
_DFLAGS?= \
	-DLIDM_VERSION=\"$(VERSION)\" \
	-DLIDM_GIT_REV=\"$(INFO_GIT_REV)\" \
	-DLIDM_BUILD_TS=$(INFO_BUILD_TS)
ALLFLAGS = $(_DFLAGS) $(CFLAGS) $(CPPFLAGS) -I$(IDIR)
LDFLAGS ?= -Wl,--gc-sections

LIBS = -lpam

# includes all headers in `$(IDIR)` and compiles everything in `$(CDIR)`
DEPS = $(wildcard $(IDIR)/*.h)
_SOURCES = $(notdir $(wildcard $(CDIR)/*.c))
OBJ = $(patsubst %.c,$(ODIR)/%.o,$(_SOURCES))

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(ALLFLAGS)

lidm: $(OBJ)
	$(CC) -o $@ $^ $(ALLFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm -f \
		$(ODIR)/* \
		lidm

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
