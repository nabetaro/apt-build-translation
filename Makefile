# $Id$

APT_BUILD = apt-build
APT_BUILD_WRAPPER = apt-build-wrapper
WRAP_PROGRAMMS = gcc cc g++ c++ make gfortran
CFLAGS = -W -Wall -Wextra -std=gnu99 -pipe -Wundef -Wshadow -Wcast-align \
	 -Wwrite-strings -Wsign-compare -Wunused -Winit-self -Wpointer-arith -Wredundant-decls \
	 -Wmissing-prototypes -Wmissing-format-attribute -Wmissing-noreturn -D_GNU_SOURCE -g
INSTALL = install
LN_S = ln -s
BINDIR = $(DESTDIR)/usr/bin
APT_BUILD_DIR = $(DESTDIR)/usr/lib/apt-build
HOST_TYPE = 

all: $(APT_BUILD_WRAPPER)
	$(MAKE) -C man/

$(APT_BUILD_WRAPPER): wrapper.c config.o
	$(CC) $(CFLAGS) -o $@ $^

install:	$(APT_BUILD_WRAPPER) $(APT_BUILD)
	$(INSTALL) -g 0 -o 0 -d $(BINDIR)
	$(INSTALL) -g 0 -o 0 -m 755 $(APT_BUILD) $(BINDIR)
	$(INSTALL) -g 0 -o 0 -d $(APT_BUILD_DIR)
	$(INSTALL) -g 0 -o 0 -m 755 $(APT_BUILD_WRAPPER) $(APT_BUILD_DIR)
	for prog in $(WRAP_PROGRAMMS); do \
		$(LN_S) $(APT_BUILD_WRAPPER) $(APT_BUILD_DIR)/$$prog; \
	done
ifneq ($(HOST_TYPE),)
	for prog in gcc g++ gfortran; do \
		$(LN_S) $(APT_BUILD_WRAPPER) $(APT_BUILD_DIR)/$(HOST_TYPE)-$$prog; \
	done
endif

clean:
	rm -f *.o $(APT_BUILD_WRAPPER) cc.wrapper.c make.wrapper.c
	$(MAKE) -C man/ clean
