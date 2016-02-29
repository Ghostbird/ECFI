# Define directories for the project.
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

# Define compilation options for the project.

# Compiler choice.
CC=gcc
#Compilation flags, includes, libraries, standard and error/warnings.
CFLAGS=-D_XOPEN_SOURCE=500 -I$(IDIR) -L$(LDIR) -L$(SHLIBDIR) -std=c99 -pedantic-errors -Wall -Wextra -Werror
# Compilation options to compile an object file (*.o) to a shared object library (*.so)
SHFLAGS=-shared -Wl,-soname,$(basename $@)
# Non-default system libraries to link.
LIBS=-lrt

# Define the tests
_TESTS=ringbuffer
TESTS=$(patsubst %,$(TBDIR)/%,$(_TESTS))

# Define the dependencies.
_DEPS=ringbuffer.h errors.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

# Define the object files to compile.
_OBJ=errors
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
$(TBDIR)/%: $(TSDIR)/%.c $(DEPS) $(OBJ) $(SHLIBDIR)/lib%.so
	mkdir -p $(TBDIR)
	$(CC) $(CFLAGS) -o $@ $< $(OBJ) $(LIBS) -l$(notdir $@)

# Generate the documentation.
doc: $(IDIR)/*.h $(SDIR)/*.c Doxyfile
	mkdir -p $(DDIR)
	doxygen

.PHONY: all tests objs shlibs runtests clean install

all:
	@echo "No application yet. Build object files with \"make $(ODIR)/<objectname>.o\""

# Compile all tests.
tests: $(TESTS) $(SHLIB)

# Compile all object files.
objs: $(OBJ)

# Compile all shared object libraries.
shlibs: $(SHLIB)

# Run all tests.
runtests: tests
	@for f in $(TBDIR)/*; do echo "==========Testing `basename $$f`=========="; LD_PRELOAD=$(SHLIB) $$f; done

# Remove all generated files.
clean:
	rm -f *~ $(IDIR)/*~ $(SDIR)/*~ $(TSDIR)/*~
	rm -rf $(DDIR) $(BDIR)
