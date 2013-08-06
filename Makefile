OCAML := ocaml
OCAMLC := ocamlc
OCAMLOPT := ocamlopt
OCAMLMKLIB := ocamlmklib

OCAML_STDLIB_DIR := $(shell ocamlc -where)
OCAML_DLL_DIR ?= $(OCAML_STDLIB_DIR)/stublibs
DEST_LIB_DIR=$(DESTDIR)/$(OCAML_STDLIB_DIR)/fontconfig
DEST_DLL_DIR=$(DESTDIR)/$(OCAML_DLL_DIR)

C_SOURCES := $(filter-out types.c, $(wildcard *.c)) types.c
C_OBJECTS := $(C_SOURCES:.c=.o)

# Always build bytecode
all: camlfontconfig.cma
install: all install-common install-byte

# Sometimes build native binaries
OCAML_HAVE_OCAMLOPT ?= $(if $(wildcard /usr/bin/ocamlopt),yes,no)
ifeq ($(OCAML_HAVE_OCAMLOPT),yes)
all: camlfontconfig.cmxa
install: install-opt
META_SOURCE := META.opt
else
META_SOURCE := META.byte
endif

.PHONY: clean all byte opt install install-common install-byte install-opt

camlfontconfig.cma: fontconfig.cmo META $(C_OBJECTS)
	$(OCAMLMKLIB) -o camlfontconfig $(C_OBJECTS) fontconfig.cmo `pkg-config --libs fontconfig` $(addprefix -ldopt ,$(LDFLAGS))

camlfontconfig.cmxa: fontconfig.cmx fontconfig.cmo META $(C_OBJECTS)
	$(OCAMLMKLIB) -o camlfontconfig $(C_OBJECTS) fontconfig.cmx fontconfig.cmo `pkg-config --libs fontconfig` $(addprefix -ldopt ,$(LDFLAGS))


%.o: %.c
	$(OCAMLC) -c $<
fontconfig.cmi: fontconfig.mli
	$(OCAMLC) -c $<
fontconfig.mli: fontconfig.ml
	$(OCAMLC) -i $< > $@
types.c fontconfig.ml: extract_consts.ml
	$(OCAML) str.cma extract_consts.ml
fontconfig.ml: fontconfig.ml.in

fontconfig.cmx: fontconfig.ml fontconfig.cmi
	$(OCAMLOPT) -c $<

fontconfig.cmo: fontconfig.ml fontconfig.cmi
	$(OCAMLC) -c $<

META: $(META_SOURCE)
	cp $< $@

clean:
	rm -f fontconfig.mli fontconfig.cmi \
	  fontconfig.cmo camlfontconfig.cma \
	  fontconfig.cmx camlfontconfig.cmxa \
	  camlfontconfig.a fontconfig.o META \
	  libcamlfontconfig.a dllcamlfontconfig.so \
	  types.c fontconfig.ml \
	  $(C_OBJECTS) \
	  test.cmi test.cmo test.cmx test.byte test.native test.o

install-common:
	install -m 755 -d $(DEST_LIB_DIR)
	install -m 755 -d $(DEST_DLL_DIR)
	install -m 644 fontconfig.cmi $(DEST_LIB_DIR)
	install -m 644 libcamlfontconfig.a $(DEST_LIB_DIR)
	install -m 644 camlfontconfig.a $(DEST_LIB_DIR)
	install -m 644 dllcamlfontconfig.so $(DEST_DLL_DIR)
	install -m 644 META $(DEST_LIB_DIR)

install-byte:
	install -m 644 camlfontconfig.cma $(DEST_LIB_DIR)

install-opt:
	install -m 644 camlfontconfig.cmxa $(DEST_LIB_DIR)

test.byte: test.ml
	ocamlc -o $@ -I . camlfontconfig.cma $<

test.native: test.ml
	ocamlopt -o $@ -I . camlfontconfig.cmxa $<
