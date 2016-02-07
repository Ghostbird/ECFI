IDIR=include
SDIR=src
DDIR=doc
ODIR=$(SDIR)/obj
LDIR=lib
TDIR=test

CC=gcc
CFLAGS=-I$(IDIR) -std=c99 -pedantic-errors -Wall -Wextra -Werror

LIBS=

_DEPS=ringbuffer.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=ringbuffer.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

all:
	@echo "No application yet. Build object files with \"make $(ODIR)/<objectname>.o\""

$(TDIR)/%: $(ODIR)/%.o $(DEPS) $(TDIR)/%.c
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean doc

doc:
	doxygen

clean:
	rm -f $(ODIR)/*.o *~ $(IDIR)/*~ $(SDIR)/*~
	rm -rf $(DDIR)/html $(DDIR)/latex
