#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <fontconfig/fontconfig.h>

#include "fccaml.h"

CAMLprim value app_font_add_dir(value dirname)
{
  CAMLparam0();
  FcBool res = FcConfigAppFontAddDir(NULL, (FcChar8 *)String_val(dirname));
  CAMLreturn(CamlFcBool(res));
}
