#include "umem_testing.h"

int main() {
  {
    umem::Host host1, host2;

    {
      size_t n = 10;
      umem::Address adr1 = host1.alloc(n+1);
      ((char*)adr1)[n] = '\0';
      adr1.set('A', n/2);
      (adr1+n/2).set('B', n/2);
      assert_str_eq((char*)adr1, "AAAAABBBBB");
      assert_is_ok(host1);
      assert_is_ok(host2);
      
      /* Same memory */
      {
        umem::Address adr2 = adr1.connect(host2, n+1);
        assert_is_ok(host1);
        assert_is_ok(host2);
        adr1.sync_to(adr2, n);
        assert_is_ok(host1);
        assert_is_ok(host2);
        assert_str_eq((char*)adr2, "AAAAABBBBB");
        (adr2+n/4).set('C', n/2);
        assert_str_eq((char*)adr2, "AACCCCCBBB");
        assert_str_eq((char*)adr1, "AACCCCCBBB"); // so, sync not necessary
        //adr1.sync_from(adr2, n);                  // but doing it anyway for testing
        adr2.sync(n);
        assert_is_ok(host1);
        assert_is_ok(host2);
        //adr1.disconnect(adr2); // Why ? This frees adr2.
        assert_str_eq((char*)adr1, "AACCCCCBBB");
      }
      
      /* Distinct memory */
      {
        umem::Address adr12 = host1.alloc(n+1);
        umem::Address adr2 = adr12.connect(host2, n+1);
        ((char*)adr2)[n] = '\0';
        adr2.set('D', n);
        assert_str_eq((char*)adr2, "DDDDDDDDDD");
        adr1.sync_to(adr2, n);
        //adr2.sync_to(n);
        assert_str_eq((char*)adr2, "AACCCCCBBB");
        adr2.set('E', n/2);
        assert_str_eq((char*)adr2, "EEEEECCBBB");
        assert_str_eq((char*)adr1, "AACCCCCBBB"); // so, need to sync
        adr1.sync_from(adr2, n);
        assert_str_eq((char*)adr1, "EEEEECCBBB");
        assert_is_ok(host1);
        assert_is_ok(host2);
      }
    }
    /* Aligned memory - different alignment */
    {
      size_t n = 1024;
      umem::Address adr1 = host1.alloc(n);
      adr1.set('A', n);
      assert_nstr_eq(10, (char*)adr1, "AAAAAAAAAA");
      size_t alignment = 1;
      while ((adr1 % alignment) == 0) alignment <<= 1;
      umem::Address adr2 = adr1.connect(host2, n, alignment);
      assert_is_ok(host1);
      assert_is_ok(host2);
      assert_eq(adr1 == adr2, false);
      assert_nstr_eq(10, (char*)adr2, "AAAAAAAAAA");
      
      (adr2+2).set('C', 4);
      assert_nstr_eq(10, (char*)adr2, "AACCCCAAAA");
      assert_nstr_eq(10, (char*)adr1, "AAAAAAAAAA");
      adr2.sync(n);
      assert_nstr_eq(10, (char*)adr1, "AACCCCAAAA");
      (adr1+3).set('D', 2);
      assert_nstr_eq(10, (char*)adr1, "AACDDCAAAA");
      assert_nstr_eq(10, (char*)adr2, "AACCCCAAAA");
      adr2.update(n);
      assert_nstr_eq(10, (char*)adr2, "AACDDCAAAA");
      assert_is_ok(host1);
      assert_is_ok(host2);
      //adr1.disconnect(adr2); // Why ?
    }
    
    /* Aligned memory - same or smaller alignment leads to same memory */
    {
      size_t n = 1024;
      size_t alignment = 128;
      umem::Address adr1 = host1.aligned_alloc(alignment, n);
      assert_is_ok(host1);
      assert_is_ok(host2);
      adr1.set('A', n);
      assert_is_ok(host1);
      assert_is_ok(host2);
      assert_nstr_eq(10, (char*)adr1, "AAAAAAAAAA");
      assert_is_ok(host1);
      assert_is_ok(host2);
      umem::Address adr2 = adr1.connect(host2, n, alignment);
      assert_is_ok(host1);
      assert_is_ok(host2);
      assert_eq(adr1 == adr2, true);

      assert_nstr_eq(10, (char*)adr2, "AAAAAAAAAA");
      (adr2+2).set('C', 4);
      assert_nstr_eq(10, (char*)adr2, "AACCCCAAAA");
      assert_nstr_eq(10, (char*)adr1, (char*)adr2);
      //adr1.sync_from(adr2, n);
      adr2.sync(n);
      assert_nstr_eq(10, (char*)adr1, (char*)adr2);
      (adr1+3).set('D', 2);
      assert_nstr_eq(10, (char*)adr1, "AACDDCAAAA");
      assert_nstr_eq(10, (char*)adr1, (char*)adr2);
      //adr1.sync_to(adr2, n);
      adr2.update(n);
      assert_nstr_eq(10, (char*)adr2, "AACDDCAAAA");
      //adr1.disconnect(adr2); // Why ?
    }
  }
  RETURN_STATUS;
}
