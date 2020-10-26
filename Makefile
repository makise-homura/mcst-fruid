BINARYNAME = mcst-fruid

OBJECTS = errors cksum-ops devtree-ops dict-ops fruid-ops i2c-ops mcst-fruid
LIBS = -li2c

PREFIX ?= /usr/local
DESTDIR ?=
LIBEXEC = $(DESTDIR)$(PREFIX)/libexec

.PHONY: all clean install uninstall

ARCH=$(shell uname -m)
ifeq ($(ARCH), e2k)
	OBJECTS += spi-ops-spidev
else
	OBJECTS += spi-ops-mtd
endif

V ?= 0
Q = $(if $(filter-out 0,$(V)),,@)
LOG = $(if $(filter-out 0,$(V)),true,echo -e)

all: $(BINARYNAME)

$(BINARYNAME): $(addsuffix .o,$(OBJECTS))
	@$(LOG) LD\\t$(BINARYNAME)
	$(Q)$(CC) -o $(BINARYNAME) $^ $(LIBS)

%.o: %.c
	@$(LOG) CC\\t$@
	$(Q)$(CC) -o $@ -c $<

clean:
	rm -f *.o $(BINARYNAME)

install:
	install -d $(LIBEXEC)
	install -m 755 $(BINARYNAME) $(LIBEXEC)

uninstall:
	rm $(LIBEXEC)/$(BINARYNAME)
