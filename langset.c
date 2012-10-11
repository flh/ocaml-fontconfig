#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <fontconfig/fontconfig.h>

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

CAMLprim value langset_make(value unit)
{
  CAMLparam0();
  CAMLlocal1(res);
  FcLangSet *fs;

  fs = FcLangSetCreate();
  res = caml_alloc_custom(&fclangset_ops, sizeof(FcLangSet *), 0, 1);
  FcLangSet_val(res) = fs;
  CAMLreturn(res);
}

CAMLprim value lang_get_all(value unit)
{
  CAMLparam0();
  CAMLlocal2(la, nla);
  FcStrList *it;
  FcStrSet *set;
  FcChar8 *lang;

  set = FcGetLangs();
  it = FcStrListCreate(set);

  la = Val_int(0); /* nil */
  while((lang = FcStrListNext(it))) {
    nla = caml_alloc(2, 0);
    Store_field(nla, 1, la);
    Store_field(nla, 0, caml_copy_string((char *)lang));
    la = nla;
  }

  FcStrListDone(it);
  FcStrSetDestroy(set);

  CAMLreturn(la);
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
