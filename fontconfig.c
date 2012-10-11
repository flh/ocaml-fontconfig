#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <fontconfig/fontconfig.h>

#include "fccaml.h"

CAMLprim value init(value unit)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcInit()));
}

CAMLprim value fini(value unit)
{
  CAMLparam0();
  FcFini();
  CAMLreturn(Val_unit);
}

CAMLprim value get_version(value unit)
{
  CAMLparam0();
  CAMLreturn(Val_int(FcGetVersion()));
}

CAMLprim value reinitialize(value unit)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcInitReinitialize()));
}

CAMLprim value bring_up_to_date(value unit)
{
  CAMLparam0();
  CAMLreturn(CamlFcBool(FcInitBringUptoDate()));
}
