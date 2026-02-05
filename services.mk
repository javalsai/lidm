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
		if [ -d /etc/sv ]; then \
			make install-service-s6; \
		elif [ -d /etc/r6nit/sv ]; then \
			make install-service-s6-etc; \
		else \
			printf '\033[31m%s\033[0m\n' "Unknown init system structure, skipping service install..." >&2; \
		fi \
	else \
		printf '\033[1;31m%s\033[0m\n' "Unknown init system, skipping service install..." >&2; \
	fi

install-service-systemd:
	@sed -e 's|ExecStart=/usr/bin/lidm|ExecStart=${DESTDIR}${PREFIX}/bin/lidm|' ./assets/services/systemd.service > ./dist/lidm.service
	install -Dm644 ./dist/lidm.service ${DESTDIR}${PREFIX}/lib/systemd/system/lidm.service
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'systemctl enable lidm'"
install-service-dinit:
	install -m644 ./assets/services/dinit ${DESTDIR}/etc/dinit.d/lidm
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'dinitctl enable lidm'"
install-service-runit:
	@if [ ! -e /etc/sv ] && [ -d /etc/runit/sv ] && [ -z "$$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "/etc/sv doesn't exist but /etc/runit/sv does" >&2; \
		printf '\033[31m%s\033[0m\n' "you probably meant to 'make install-service-runit-etc'" >&2; \
		exit 1; \
	fi
	mkdir -p ${DESTDIR}/etc/sv/lidm
	cp -r --update=all ./assets/services/runit/* ${DESTDIR}/etc/sv/lidm/
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'ln -s ${DESTDIR}/etc/sv/lidm /var/service' or your distro equivalent"
install-service-runit-etc:
	@if [ ! -e /etc/runit/sv ] && [ -d /etc/sv ] && [ -z "$$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "/etc/runit/sv doesn't exist but /etc/sv does" >&2; \
		printf '\033[31m%s\033[0m\n' "you probably meant to 'make install-service-runit'" >&2; \
		exit 1; \
	fi
	mkdir -p ${DESTDIR}/etc/runit/sv/lidm
	cp -r --update=all ./assets/services/runit/* ${DESTDIR}/etc/runit/sv/lidm/
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'ln -s ${DESTDIR}/etc/runit/sv/lidm /run/runit/service' or your distro equivalent"
install-service-openrc:
	install -m755 ./assets/services/openrc ${DESTDIR}/etc/init.d/lidm
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 'rc-update add lidm'"
install-service-s6:
	@if [ ! -e /etc/sv ] && [ -d /etc/s6/sv ] && [ -z "$$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "/etc/sv doesn't exist but /etc/s6/sv does" >&2; \
		printf '\033[31m%s\033[0m\n' "you probably meant to 'make install-service-s6-etc'" >&2; \
		exit 1; \
	fi
	mkdir -p ${DESTDIR}/etc/sv/lidm
	cp -r --update=all ./assets/services/s6/* ${DESTDIR}/etc/sv/lidm/
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 's6-service add default lidm' and 's6-db-reload'"
install-service-s6-etc:
	@if [ ! -e /etc/s6/sv ] && [ -d /etc/sv ] && [ -z "$$FORCE" ]; then \
		printf '\033[31m%s\033[0m\n' "/etc/s6/sv doesn't exist but /etc/sv does" >&2; \
		printf '\033[31m%s\033[0m\n' "you probably meant to 'make install-service-s6'" >&2; \
		exit 1; \
	fi
	mkdir -p ${DESTDIR}/etc/s6/sv/lidm
	cp -r --update=all ./assets/services/s6/* ${DESTDIR}/etc/s6/sv/lidm/
	@printf '\033[1m%s\033[0m\n\n' " don't forget to run 's6-service add default lidm' and 's6-db-reload'"

uninstall-service:
	rm -rf \
		${DESTDIR}${PREFIX}/lib/systemd/system/lidm.service \
		${DESTDIR}/etc/dinit.d/lidm \
		${DESTDIR}/etc/sv/lidm \
		${DESTDIR}/etc/runit/sv/lidm \
		${DESTDIR}/etc/init.d/lidm \
		${DESTDIR}/etc/s6/sv/lidm

.PHONY: install-service uninstall-service \
	install-service-s6 \
	install-service-s6-etc \
	install-service-dinit \
	install-service-runit \
	install-service-runit-etc \
	install-service-openrc \
	install-service-systemd
