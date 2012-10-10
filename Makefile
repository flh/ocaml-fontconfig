OCAML=ocaml
OCAMLC=ocamlc
OCAMLMKLIB=ocamlmklib

.PHONY: clean all test

all: fontconfig.cma

fontconfig.cma: fontconfig.cmo fontconfig.o
	$(OCAMLMKLIB) -o fontconfig fontconfig.cmo fontconfig.o `pkg-config --libs fontconfig`

fontconfig.o: fontconfig.c types.h
	$(OCAMLC) -c $<
fontconfig.cmi: fontconfig.mli
	$(OCAMLC) -c $<
fontconfig.cmo: fontconfig.ml fontconfig.cmi
	$(OCAMLC) -c $<
fontconfig.mli: fontconfig.ml
	$(OCAMLC) -i $< > $@
types.h fontconfig.ml: extract_consts.ml
	$(OCAML) str.cma extract_consts.ml
fontconfig.ml: fontconfig.ml.in

test:
	$(OCAML) fontconfig.cma test.ml

clean:
	rm -f fontconfig.mli fontconfig.cmi fontconfig.cmo fontconfig.o fontconfig.a \
	  libfontconfig.a fontconfig.cma dllfontconfig.so \
	  types.h fontconfig.ml
