#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <fontconfig/fontconfig.h>

#include "fccaml.h"

/* LangSets */

/* Custom type for OCaml, boxing the FcLangSet struct */
/* Accessing the FcLangSet part of an OCaml custom block */
#define FcLangSet_val(v) (*((FcLangSet **) Data_custom_val(v)))

void fclangset_finalize(value v)
{
  FcLangSetDestroy(FcLangSet_val(v));
}

long fclangset_hash(value v)
{
  long hash = 0;
  hash += FcLangSetHash(FcLangSet_val(v));
  return hash;
}

static struct custom_operations fclangset_ops =
{
    "fr.freedesktop.org.Software.fontconfig.FcLangSet",
    &fclangset_finalize,
    custom_compare_default,
    &fclangset_hash,
    custom_serialize_default,
    custom_deserialize_default
};

value caml_make_langset(FcLangSet *ls)
{
  CAMLparam0();
  CAMLlocal1(res);

  res = caml_alloc_custom(&fclangset_ops, sizeof(FcLangSet *), 0, 1);
  FcLangSet_val(res) = ls;

  CAMLreturn(res);
}

CAMLprim value langset_make(value unit)
{
  CAMLparam0();
  CAMLreturn(caml_make_langset(FcLangSetCreate()));
}

CAMLprim value langset_copy(value ls)
{
  CAMLparam0();
  CAMLreturn(caml_make_langset(FcLangSetCopy(FcLangSet_val(ls))));
}

CAMLprim value langset_add(value ls, value lang)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcLangSetAdd(FcLangSet_val(ls), (FcChar8 *)String_val(lang))));
}

CAMLprim value langset_del(value ls, value lang)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcLangSetDel(FcLangSet_val(ls), (FcChar8 *)String_val(lang))));
}

CAMLprim value langset_union(value lsa, value lsb)
{
  CAMLparam0();
  CAMLreturn(caml_make_langset(FcLangSetUnion(FcLangSet_val(lsa), FcLangSet_val(lsb))));
}

CAMLprim value langset_subtract(value lsa, value lsb)
{
  CAMLparam0();
  CAMLreturn(caml_make_langset(FcLangSetSubtract(FcLangSet_val(lsa), FcLangSet_val(lsb))));
}

CAMLprim value langset_compare(value la, value lb)
{
  CAMLparam0();
  CAMLlocal1(res);

  switch(FcLangSetCompare(FcLangSet_val(la), FcLangSet_val(lb)))
  {
    case FcLangEqual:
      res = Val_int(0);
      break;
    case FcLangDifferentTerritory:
      res = Val_int(1);
      break;
    case FcLangDifferentLang:
      res = Val_int(2);
      break;
  }

  CAMLreturn(res);
}

CAMLprim value langset_contains(value lsa, value lsb)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcLangSetContains(FcLangSet_val(lsa), FcLangSet_val(lsb))));
}

CAMLprim value langset_equal(value lsa, value lsb)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcLangSetEqual(FcLangSet_val(lsa), FcLangSet_val(lsb))));
}

CAMLprim value langset_haslang(value ls, value lang)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcLangSetHasLang(FcLangSet_val(ls), String_val(lang))));
}

CAMLprim value fcstrset_to_caml(FcStrSet *set)
{
  CAMLparam0();
  CAMLlocal2(res, nres);
  FcStrList *iter = FcStrListCreate(set);
  char *str;

  res = Val_int(0); /* nil */

  while((str = FcStrListNext(iter))) {
    nres = caml_alloc(2, 0);
    Store_field(nres, 0, caml_copy_string(str));
    Store_field(nres, 1, res);
    res = nres;
  }

  FcStrListDone(iter);

  CAMLreturn(res);
}

CAMLprim value langset_defaultlangs(value unit)
{
  CAMLparam0();
  CAMLlocal1(res);
#if FC_VERSION >= 21000
  FcStrSet *langs = FcGetDefaultLangs();
  res = fcstrset_to_caml(langs);
  FcStrSetDestroy(langs);
#else
  res = Val_int(0); /* nil */
#endif
  CAMLreturn(res);
}

CAMLprim value langset_get_all(value unit)
{
  CAMLparam0();
  CAMLlocal1(la);
  FcStrSet *set = FcGetLangs();
  la = fcstrset_to_caml(set);
  FcStrSetDestroy(set);
  CAMLreturn(la);
}
