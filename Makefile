OCAML=ocaml
OCAMLC=ocamlc -g
OCAMLOPT=ocamlopt
OCAMLMKLIB=ocamlmklib

OCAML_STDLIB_DIR=$(shell ocamlc -where)
OCAML_DLL_DIR ?= $(OCAML_STDLIB_DIR)/stublibs
DEST_LIB_DIR=$(DESTDIR)/$(OCAML_STDLIB_DIR)/fontconfig
DEST_DLL_DIR=$(DESTDIR)/$(OCAML_DLL_DIR)

C_SOURCES=$(filter-out types.c, $(wildcard *.c)) types.c
C_OBJECTS=$(C_SOURCES:.c=.o)

OCAML_HAVE_OCAMLOPT ?= $(if $(wildcard /usr/bin/ocamlopt),yes,no)
ifeq ($(OCAML_HAVE_OCAMLOPT),yes)
  OPT_FLAG=opt
endif

.PHONY: clean all byte opt install install-common install-byte install-opt

all: byte $(OPT_FLAG)
byte: fontconfig.cma META
opt:  fontconfig.cmxa META

fontconfig.cma: fontconfig.cmo $(C_OBJECTS)
	$(OCAMLMKLIB) -o fontconfig $< $(C_OBJECTS) `pkg-config --libs fontconfig` $(addprefix -ldopt,$(LDFLAGS))

fontconfig.cmxa: fontconfig.cmx $(C_OBJECTS)
	$(OCAMLMKLIB) -o fontconfig $< $(C_OBJECTS) `pkg-config --libs fontconfig` $(addprefix -ldopt,$(LDFLAGS))

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

ifeq ($(OPT_FLAG),opt)
  META_SOURCE=META.opt
else
  META_SOURCE=META.byte
endif
META: $(META_SOURCE)
	cp $< $@

clean:
	rm -f fontconfig.mli fontconfig.cmi \
	  fontconfig.cmo fontconfig.cma \
	  fontconfig.cmx fontconfig.cmxa \
	  fontconfig.a META \
	  libfontconfig.a fontconfig.cma dllfontconfig.so \
	  types.c fontconfig.ml \
	  $(C_OBJECTS)

install: all install-byte $(addprefix install-,$(OPT_FLAG))

install-common:
	install -m 755 -d $(DEST_LIB_DIR)
	install -m 755 -d $(DEST_DLL_DIR)
	install -m 644 fontconfig.cmi $(DEST_LIB_DIR)
	install -m 644 libfontconfig.a $(DEST_LIB_DIR)
	install -m 644 fontconfig.a $(DEST_LIB_DIR)
	install -m 644 dllfontconfig.so $(DEST_DLL_DIR)
	install -m 644 META $(DEST_LIB_DIR)

install-byte: install-common byte
	install -m 644 fontconfig.cma $(DEST_LIB_DIR)

install-opt: install-common opt
	install -m 644 fontconfig.cmxa $(DEST_LIB_DIR)
