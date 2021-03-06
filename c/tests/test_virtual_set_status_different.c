#include "umem_testing.h"

int main() {
  umemVirtual virt;
  umemVirtual_ctor(&virt, NULL);
  umem_set_status(&virt, umemNotImplementedError, "notimpl");
  umem_set_status(&virt, umemAssertError, "assert");
  assert(umem_get_status(&virt) == umemAssertError);
  assert_str_eq(umem_get_message(&virt), "notimpl\nstatus NotImplementedError changed to AssertError\nassert");
  assert_is_not_ok(virt);
  umemVirtual_dtor(&virt);
  RETURN_STATUS;
}
