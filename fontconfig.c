#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <fontconfig/fontconfig.h>

CAMLprim value init(value unit)
{
  CAMLparam1(unit);
  CAMLreturn(FcInit() == FcTrue ? Val_true : Val_false);
}

CAMLprim value fini(value unit)
{
  CAMLparam1(unit);
  FcFini();
  CAMLreturn(Val_unit);
}

CAMLprim value get_version(value unit)
{
  CAMLparam1(unit);
  CAMLreturn(Val_int(FcGetVersion()));
}

CAMLprim value reinitialize(value unit)
{
  CAMLparam1(unit);
  CAMLreturn(FcInitReinitialize() == FcTrue ? Val_true : Val_false);
}

CAMLprim value bring_up_to_date(value unit)
{
  CAMLparam1(unit);
  CAMLreturn(FcInitBringUptoDate() == FcTrue ? Val_true : Val_false);
}
