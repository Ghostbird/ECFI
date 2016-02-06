IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -std=c99 -pedantic

ODIR=obj
LDIR =../lib

LIBS=

_DEPS = ringbuffer.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = ringbuffer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)


all:
	@echo "No application yet. Build object files with \"make obj/<objectname>.o\""

doc:
	cd .. && doxygen

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(INCDIR)/*~
