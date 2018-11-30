#include "umem_testing.h"

int main() {
  umemCuda cuda;
  umemCuda_ctor(&cuda, 0);
  assert_is_ok(cuda);
  uintptr_t addr = umem_alloc(&cuda, 10);
  assert_is_ok(cuda);
  umem_free(&cuda, addr);
  assert_is_ok(cuda);
  umemCuda_dtor(&cuda);
  assert_is_ok(cuda);
  RETURN_STATUS;
}