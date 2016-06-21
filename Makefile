# Define directories for the project.
IDIR=include
ADIR=asm
BDIR=bin
SDIR=src
DDIR=doc
ODIR=$(BDIR)/obj
LDIR=lib
SHLIBDIR=$(BDIR)/lib
TDIR=test
TSDIR=$(SDIR)/$(TDIR)
TBDIR=$(BDIR)/$(TDIR)

# Define compilation options for the project.

# Compiler choice.
CC?=gcc
#Compilation flags, includes, libraries, standard and error/warnings.
CFLAGS=-D_XOPEN_SOURCE=500 -I$(IDIR) -L$(LDIR) -L$(SHLIBDIR) -std=c99 -pedantic-errors -Wall -Wextra -Werror $(DEBUG)
# Compilation options to compile an object file (*.o) to a shared object library (*.so)
SHFLAGS=-shared -Wl,-soname,$(basename $@)
# Non-default system libraries to link.
LIBS=-lrt

# Define the tests
_TESTS=ringbuffer read-speed
TESTS=$(patsubst %,$(TBDIR)/%,$(_TESTS))

# Define the dependencies.
_DEPS=ringbuffer.h errors.h cfi-heuristics.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

# Define the object files to compile.
_OBJ=errors cfi-heuristics
OBJ=$(patsubst %,$(ODIR)/%.o,$(_OBJ))

# Define the shared object libraries to compile.
_SHLIB=ringbuffer
SHLIB=$(patsubst %,$(SHLIBDIR)/lib%.so,$(_SHLIB))

# Compile an object file from a source file
$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -fPIC -o $@ $< $(LIBS)

# Compile a shared object library from an object file.
$(SHLIBDIR)/lib%.so: $(ODIR)/%.o
	mkdir -p $(SHLIBDIR)
	$(CC) $(SHFLAGS) -o $@ $^

# Compile a test from a test source file.
$(TBDIR)/%: $(TSDIR)/%.c $(DEPS) $(OBJ) $(SHLIB)
	mkdir -p $(TBDIR)
	$(CC) $(CFLAGS) -o $@ $< $(OBJ) $(LIBS) $(patsubst %,-l%,$(notdir $(_SHLIB)))

$(BDIR)/%: $(SDIR)/%.c $(IDIR)/%.h $(DEPS) $(OBJ) $(SHLIB)
	mkdir -p $(BDIR)
	$(CC) $(CFLAGS) -o $@ $< $(DEPS) $(OBJ) $(LIBS) $(patsubst %,-l%,$(notdir $(_SHLIB)))

$(ADIR)/%.s: $(SDIR)/%.c $(IDIR)/%.h $(DEPS) $(OBJ) $(SHLIB)
	mkdir -p $(ADIR)
	$(CC) $(CFLAGS) -fverbose-asm -S $< $(LIBS) $(patsubst %,-l%,$(notdir $(_SHLIB)))
	mv $(notdir $@) $@

bin/BOF4: CFLAGS=-D_XOPEN_SOURCE=500 -I$(IDIR) -L$(LDIR) -L$(SHLIBDIR) -Wall $(DEBUG)

asm/BOF4.s: CFLAGS=-D_XOPEN_SOURCE=500 -I$(IDIR) -L$(LDIR) -L$(SHLIBDIR) -Wall $(DEBUG)

.PHONY: all tests objs shlibs runtests clean install doc debug runbof

all: bin/cfi-checker tests

# Generate the documentation.
doc: $(IDIR)/*.h $(SDIR)/*.c Doxyfile
	mkdir -p $(DDIR)
	doxygen

# Compile all tests.
tests: $(TESTS) $(SHLIB) bin/injection-test

# Compile all object files.
objs: $(OBJ)

# Compile all shared object libraries.
shlibs: $(SHLIB)

debug: DEBUG = -ggdb
debug: all


# Run all tests.
runtests: tests
	@for f in $(TBDIR)/*; do echo "==========Testing `basename $$f`=========="; LD_PRELOAD=$(SHLIB) $$f; done

valgrind: tests
	@for f in $(TBDIR)/*; do echo "==========Running Valgrind on `basename $$f`=========="; LD_PRELOAD=$(SHLIB) LD_PRELOAD=$(SHLIB) valgrind --leak-check=full $$f; done

runbof: bin/BOF4 asm/BOF4.s bin/cfi-checker
	LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker bin/BOF4

# Remove all generated files.
clean:
	rm -f *~ $(IDIR)/*~ $(SDIR)/*~ $(TSDIR)/*~
	rm -rf $(DDIR)/html $(DDIR)/latex $(BDIR)
