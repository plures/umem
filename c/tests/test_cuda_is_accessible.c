#include "umem_testing.h"

int main() {
  umemHost host;
  umemCuda cuda1, cuda12;
  umemHost_ctor(&host);
  umemCuda_ctor(&cuda1, 0);
  umemCuda_ctor(&cuda12, 0);
  assert_is_ok(host);
  assert_is_ok(cuda1);
  assert_is_ok(cuda12);
  assert_eq(umem_is_accessible_from(&cuda1, &host), false);
  assert_eq(umem_is_accessible_from(&cuda1, &cuda1), true);
  assert_eq(umem_is_accessible_from(&cuda1, &cuda12), true);
  assert_is_ok(host);
  assert_is_ok(cuda1);
  assert_is_ok(cuda12);
  umem_dtor(&host);
  umem_dtor(&cuda1);
  umem_dtor(&cuda12);
  RETURN_STATUS;
}
