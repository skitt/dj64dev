TOP ?= .
PREFIX ?= /usr/local
INSTALL ?= install

VERSION = 0.1
DJLIBC = $(TOP)/lib/libc.a
DJ64LIB = $(TOP)/lib/libdj64.so.0.1
DJ64DEVL = $(TOP)/lib/libdj64.so

all: dj64.pc
	$(MAKE) -C src

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib
	$(INSTALL) -m 0644 $(DJLIBC) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	$(INSTALL) $(DJ64LIB) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	cp -fP $(DJ64DEVL) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/include
	cp -r $(TOP)/include $(DESTDIR)$(PREFIX)/i386-pc-dj64
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	$(INSTALL) -m 0644 dj64.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig

uninstall:
	$(RM) -r $(DESTDIR)$(PREFIX)/i386-pc-dj64

clean:
	$(MAKE) -C src clean
	$(RM) dj64.pc

deb:
	debuild -i -us -uc -b

%.pc: %.pc.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@INCLUDEDIR[@]!$(INCLUDEDIR)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		-e 's!@VERSION[@]!$(VERSION)!g' \
		$< >$@
