IDIR=include
BDIR=bin
SDIR=src
DDIR=doc
ODIR=$(BDIR)/obj
LDIR=lib
SHLIBDIR=$(BDIR)/lib
TDIR=test
TSDIR=$(SDIR)/$(TDIR)
TBDIR=$(BDIR)/$(TDIR)

CC=gcc
CFLAGS=-D_XOPEN_SOURCE=500 -I$(IDIR) -L$(LDIR) -L$(SHLIBDIR) -std=c99 -pedantic-errors -Wall -Wextra -Werror
SHFLAGS=-shared -Wl,-soname,$(basename $@)

LIBS=-lrt

_TESTS=ringbuffer
TESTS=$(patsubst %,$(TBDIR)/%,$(_TESTS))

_DEPS=ringbuffer.h errors.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=errors
OBJ=$(patsubst %,$(ODIR)/%.o,$(_OBJ))

_SHLIB=ringbuffer
SHLIB=$(patsubst %,$(SHLIBDIR)/lib%.so,$(_SHLIB))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -fPIC -o $@ $< $(LIBS)

$(SHLIBDIR)/lib%.so: $(ODIR)/%.o
	mkdir -p $(SHLIBDIR)
	$(CC) $(SHFLAGS) -o $@ $^

$(TBDIR)/%: $(TSDIR)/%.c $(DEPS) $(OBJ) $(SHLIBDIR)/lib%.so
	mkdir -p $(TBDIR)
	$(CC) $(CFLAGS) -o $@ $< $(OBJ) $(LIBS) -l$(notdir $@)

doc: $(IDIR)/*.h $(SDIR)/*.c Doxyfile
	mkdir -p $(DDIR)
	doxygen

.PHONY: all tests objs shlibs runtests clean install

all:
	@echo "No application yet. Build object files with \"make $(ODIR)/<objectname>.o\""

tests: $(TESTS) $(SHLIB)

objs: $(OBJ)

shlibs: $(SHLIB)

runtests: tests
	@for f in $(TBDIR)/*; do echo "==========Testing `basename $$f`=========="; LD_PRELOAD=$(SHLIB) $$f; done

clean:
	rm -f *~ $(IDIR)/*~ $(SDIR)/*~ $(TSDIR)/*~
	rm -rf $(DDIR) $(BDIR)
