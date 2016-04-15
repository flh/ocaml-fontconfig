#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <fontconfig/fontconfig.h>

#include "fccaml.h"

/* Matrix conversions from/to Caml, defined in matrix.c */
FcMatrix *fcmatrix_from_caml(value m);
value caml_from_fcmatrix(const FcMatrix *m);

/* Constant arrays defined in types.c */
extern const int fcSlant[];
extern const int fcWidth[];
extern const int fcWeight[];

/* Custom type for OCaml, boxing the FcPattern struct */
/* Accessing the FcPattern part of an OCaml custom block */
#define FcPattern_val(v) (*((FcPattern **) Data_custom_val(v)))

void fcpattern_finalize(value v)
{
  FcPatternDestroy(FcPattern_val(v));
}

long fcpattern_hash(value v)
{
  long hash = 0;
  hash += FcPatternHash(FcPattern_val(v));
  return hash;
}

static struct custom_operations fcpattern_ops =
{
    "fr.freedesktop.org.Software.fontconfig.FcPattern",
    &fcpattern_finalize,
    custom_compare_default,
    &fcpattern_hash,
    custom_serialize_default,
    custom_deserialize_default
};


FcValue fcvalue_from_caml(value c)
{
  FcValue res;
  if(Is_block(c)) {
    switch(Tag_val(c)) {
      case 0: /* Integer */
        res.type = FcTypeInteger;
        res.u.i = Int_val(Field(c, 0));
        break;
      case 1: /* Double */
        res.type = FcTypeDouble;
        res.u.d = Double_val(Field(c, 0));
        break;
      case 2: /* String */
        res.type = FcTypeString;
        res.u.s = FcStrCopy((FcChar8 *)String_val(Field(c, 0)));
        break;
      case 3: /* Bool */
        res.type = FcTypeBool;
        res.u.b = Bool_val(Field(c, 0)) ? FcTrue : FcFalse;
        break;
      case 4: /* Matrix */
        res.type = FcTypeMatrix;
        res.u.m = fcmatrix_from_caml(Field(c, 0));
        break;
    }
  }
  else {
    /* C'est void */
    res.type = FcTypeVoid;
  }
  return res;
}

value caml_from_fcvalue(FcValue v)
{
  CAMLparam0();
  CAMLlocal2(res, arr);
  switch(v.type) {
    case FcTypeVoid:
      res = Val_int(0);
      break;
    case FcTypeInteger:
      res = caml_alloc(1, 0);
      Store_field(res, 0, Val_int(v.u.i));
      break;
    case FcTypeDouble:
      res = caml_alloc(1, 1);
      Store_field(res, 0, caml_copy_double(v.u.d));
      break;
    case FcTypeString:
      res = caml_alloc(1, 2);
      Store_field(res, 0, caml_copy_string((char *)v.u.s));
      break;
    case FcTypeBool:
      res = caml_alloc(1, 3);
      Store_field(res, 0, v.u.b ? Val_true : Val_false);
      break;
    case FcTypeMatrix:
      res = caml_from_fcmatrix(v.u.m);
    default:
      /* caml_invalid_argument ? */
      break;
  }
  CAMLreturn(res);
}

/* Module Pattern */
value caml_box_fcpattern(FcPattern *pat)
{
  CAMLparam0();
  CAMLlocal1(res);
  res = caml_alloc_custom(&fcpattern_ops, sizeof(FcPattern *), 0, 1);
  FcPattern_val(res) = pat;
  CAMLreturn(res);
}

value caml_copy_pattern(FcPattern *pat)
{
  CAMLparam0();
  CAMLlocal1(res);
  CAMLreturn(caml_box_fcpattern(FcPatternDuplicate(pat)));
}

CAMLprim value pattern_make(value props)
{
  CAMLparam0();
  CAMLreturn(caml_box_fcpattern(FcPatternCreate()));
}

CAMLprim value pattern_get(value pat, value prop, value id)
{
  CAMLparam0();
  CAMLlocal1(res);
  FcResult result;
  FcValue val;
  result = FcPatternGet(FcPattern_val(pat), String_val(prop), Int_val(id), &val);
  switch(result) {
    case FcResultMatch:
      res = caml_from_fcvalue(val);
      break;
    case FcResultNoId:
      caml_invalid_argument("pattern object id");
      break;
    default:
      caml_invalid_argument("pattern object unsupported type");
      break;
  }
  CAMLreturn(res);
}

CAMLprim value pattern_add(value pat, value prop, value append)
{
  CAMLparam0();
  FcPatternAdd(FcPattern_val(pat),
      String_val(Field(prop, 0)),
      fcvalue_from_caml(Field(prop, 1)),
      Bool_val(append) ? FcTrue : FcFalse);
  CAMLreturn(Val_unit);
}

CAMLprim value pattern_addweak(value pat, value prop, value append)
{
  CAMLparam0();
  FcPatternAddWeak(FcPattern_val(pat),
      String_val(Field(prop, 0)),
      fcvalue_from_caml(Field(prop, 1)),
      Bool_val(append) ? FcTrue : FcFalse);
  CAMLreturn(Val_unit);
}

CAMLprim value pattern_del(value pat, value prop)
{
  CAMLparam0();
  FcBool res = FcPatternDel(FcPattern_val(pat),
      String_val(prop));
  CAMLreturn(CamlFcBool(res));
}

CAMLprim value pattern_copy(value pat)
{
  CAMLparam0();
  CAMLreturn(caml_copy_pattern(FcPattern_val(pat)));
}

CAMLprim value pattern_remove(value pat, value prop, value pos)
{
  CAMLparam0();
  FcBool res = FcPatternRemove(FcPattern_val(pat),
      String_val(prop),
      Int_val(pos));
  CAMLreturn(CamlFcBool(res));
}

CAMLprim value default_substitute(value pat)
{
  CAMLparam0();
  FcDefaultSubstitute(FcPattern_val(pat));
  CAMLreturn(Val_unit);
}

CAMLprim value pattern_format(value pat, value format)
{
  CAMLparam0();
  CAMLlocal1(res);

  char *str_res = (char *)FcPatternFormat(FcPattern_val(pat),
        (FcChar8 *) String_val(format));

  if(str_res == NULL) {
    caml_invalid_argument("pattern format");
  }
  else {
    res = caml_copy_string(str_res);
    free(str_res);
  }

  CAMLreturn(res);
}

CAMLprim value slant_to_int(value v)
{
  CAMLparam0();
  CAMLreturn(Val_int(fcSlant[Int_val(v)]));
}

CAMLprim value width_to_int(value v)
{
  CAMLparam0();
  CAMLreturn(Val_int(fcWidth[Int_val(v)]));
}

CAMLprim value weight_to_int(value v)
{
  CAMLparam0();
  CAMLreturn(Val_int(fcWeight[Int_val(v)]));
}

CAMLprim value pattern_find_font(value plist)
{
  CAMLparam0();
  CAMLlocal1(res);
  FcPattern *pat, *match;
  FcResult result = FcResultMatch;

  pat = FcPattern_val(plist);
  FcConfigSubstitute(NULL, pat, FcMatchPattern);
  FcDefaultSubstitute(pat); 
  match = FcFontMatch(NULL, pat, &result);

  if(result == FcResultMatch) {
    res = caml_box_fcpattern(match);
  }
  else {
    caml_raise_constant(*caml_named_value("not_found exception"));
  }

  CAMLreturn(res);
}

CAMLprim value pattern_font_sort(value plist, value trim)
{
  CAMLparam0();
  CAMLlocal2(res, nres);
  FcPattern *pat;
  FcFontSet *match;
  FcResult result;
  int i;

  pat = FcPattern_val(plist);
  FcConfigSubstitute(NULL, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);

  match = FcFontSort(NULL, pat, Bool_val(trim) ? FcTrue : FcFalse, NULL, &result);

  /* Reconstruire la belle liste */
  res = Val_int(0); /* empty list */
  for(i = match->nfont; i >= 0; i--) {
    nres = caml_alloc(2, 0);
    Store_field(nres, 0, caml_copy_pattern(match->fonts[i]));
    Store_field(nres, 1, res);
    res = nres;
  }

  FcFontSetDestroy(match);
  FcPatternDestroy(pat);
  CAMLreturn(res);
}
