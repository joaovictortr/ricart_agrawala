SUBDIRS = src
DOXYGEN = /usr/bin/doxygen

.PHONY: all clean doc src

all: src

src:
	$(MAKE) -C src/

doc:
	$(DOXYGEN) doxyfile

clean:
	$(MAKE) -C src/ clean
	@rm -rf doc/html doc/latex

