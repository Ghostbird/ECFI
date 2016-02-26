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
SHFLAGS=-shared -Wl,-soname,$(basename $@)

LIBS=-lrt

_TESTS=ringbuffer
TESTS=$(patsubst %,$(TBDIR)/%,$(_TESTS))

_DEPS=ringbuffer.h errors.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=ringbuffer errors
OBJ=$(patsubst %,$(ODIR)/%.o,$(_OBJ))

SHLIBS=$(patsubst %,$(ODIR)/lib%.so,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -fPIC -o $@ $< $(LIBS)

$(ODIR)/lib%.so: $(ODIR)/%.o
	$(CC) $(SHFLAGS) -o $@ $^

$(TBDIR)/%: $(TSDIR)/%.c $(OBJ) $(DEPS)
	mkdir -p $(TBDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

doc: $(IDIR)/*.h $(SDIR)/*.c Doxyfile
	mkdir -p $(DDIR)
	doxygen

.PHONY: all tests objs shlibs runtests clean

all:
	@echo "No application yet. Build object files with \"make $(ODIR)/<objectname>.o\""

tests: $(TESTS)

objs: $(OBJ)

shlibs: $(SHLIBS)

runtests: tests
	@for f in $(TBDIR)/*; do echo "==========Testing `basename $$f`=========="; $$f; done

clean:
	rm -f *~ $(IDIR)/*~ $(SDIR)/*~ $(TSDIR)/*~
	rm -rf $(DDIR) $(BDIR)
