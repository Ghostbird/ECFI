IDIR=include
BDIR=bin
SDIR=src
DDIR=doc
ODIR=$(BDIR)/obj
LDIR=lib
TDIR=test
TSDIR=$(SDIR)/$(TDIR)
TBDIR=$(BDIR)/$(TDIR)

CC=gcc
CFLAGS=-D_XOPEN_SOURCE=500 -I$(IDIR) -std=c99 -pedantic-errors -Wall -Wextra -Werror

LIBS=-lrt

_TESTS=ringbuffer
TESTS=$(patsubst %,$(TBDIR)/%,$(_TESTS))

_DEPS=ringbuffer.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=ringbuffer.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(TBDIR)/%: $(ODIR)/%.o $(DEPS) $(TSDIR)/%.c
	mkdir -p $(TBDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

doc: $(IDIR)/*.h $(SDIR)/*.c Doxyfile
	mkdir -p $(DDIR)
	doxygen

.PHONY: all tests objs runtests clean

all:
	@echo "No application yet. Build object files with \"make $(ODIR)/<objectname>.o\""

tests: $(TESTS)

objs: $(OBJ)

runtests: tests
	@for f in $(TBDIR)/*; do echo "==========Testing `basename $$f`=========="; $$f; done

clean:
	rm -f *~ $(IDIR)/*~ $(SDIR)/*~ $(TSDIR)/*~
	rm -rf $(DDIR) $(BDIR)
