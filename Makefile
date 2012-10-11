OCAML=ocaml
OCAMLC=ocamlc
OCAMLMKLIB=ocamlmklib
C_SOURCES=types.c $(wildcard *.c)
C_OBJECTS=$(C_SOURCES:.c=.o)

.PHONY: clean all test

all: fontconfig.cma

fontconfig.cma: fontconfig.cmo $(C_OBJECTS)
	$(OCAMLMKLIB) -o fontconfig fontconfig.cmo $(C_OBJECTS) `pkg-config --libs fontconfig`

%.o: %.c
	$(OCAMLC) -c $<
fontconfig.cmi: fontconfig.mli
	$(OCAMLC) -c $<
fontconfig.cmo: fontconfig.ml fontconfig.cmi
	$(OCAMLC) -c $<
fontconfig.mli: fontconfig.ml
	$(OCAMLC) -i $< > $@
types.c fontconfig.ml: extract_consts.ml
	$(OCAML) str.cma extract_consts.ml
fontconfig.ml: fontconfig.ml.in

test:
	$(OCAML) fontconfig.cma test.ml

clean:
	rm -f fontconfig.mli fontconfig.cmi fontconfig.cmo fontconfig.a \
	  libfontconfig.a fontconfig.cma dllfontconfig.so \
	  types.c fontconfig.ml \
	  $(C_OBJECTS)
