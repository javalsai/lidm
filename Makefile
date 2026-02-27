VERSION := 2.0.2
.DEFAULT_GOAL := lidm

CDIR = src
LDIR = lib
IDIR = include
ODIR = dist

PREFIX = /usr/local

INFO_GIT_REV != git describe --long --tags --always 2>/dev/null || echo '?'
INFO_BUILD_TS != date +%s

CFLAGS ?= -O3 -Wall -Wextra -fdata-sections -ffunction-sections
# C PreProcessor flags, not C Plus Plus
CPPFLAGS ?=
_DFLAGS?= \
	-DLIDM_VERSION=\"$(VERSION)\" \
	-DLIDM_GIT_REV=\"$(INFO_GIT_REV)\" \
	-DLIDM_BUILD_TS=$(INFO_BUILD_TS)
ALLFLAGS = $(_DFLAGS) $(CFLAGS) $(CPPFLAGS) -I$(IDIR)
LDFLAGS ?= -Wl,--gc-sections -L${PREFIX}/lib

LIBS = -lpam

# includes all headers in `$(IDIR)` and compiles everything in `$(CDIR)`
HEADERS != find $(IDIR) -type f -name "*.h"
SOURCES != find $(CDIR) -type f -name "*.c"
ALL_OBJECTS = ${SOURCES:src/%.c=$(ODIR)/%.o}

lidm: $(ALL_OBJECTS)
	$(CC) -o $@ $^ $(ALLFLAGS) $(LIBS) $(LDFLAGS)

.for src in $(SOURCES)
object = $(ODIR)/${src:S|^$(CDIR)/||:R}.o
${object}: ${src} ${HEADERS}
	@mkdir -p ${.TARGET:H}
	@echo "DEBUG: compilation de $(src), cible [${.TARGET}] générée à partir de [${.ALLSRC:M*.c}]"
	$(CC) $(ALLFLAGS) -c -o ${.TARGET} ${src}
.endfor

clean:
	rm -rf $(ODIR) lidm

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
