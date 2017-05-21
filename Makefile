default: build

build:
	./elf_strings.py
	gcc -Wall -O3 main.c -ljson-c

install:
	mkdir -p ${DESTDIR}/usr/bin
	cp a.out ${DESTDIR}/usr/bin/jsonelf

uninstall:
	rm ${DESTDIR}/usr/bin/jsonelf

clean:
	rm -f a.out elf_strings.h
