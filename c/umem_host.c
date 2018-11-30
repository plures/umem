#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "umem.h"

#define HOST_CALL(ME, CALL, ERROR, ERRRETURN, FMT, ...)	\
  do {							\
    if ((void*)(CALL) == NULL) {			\
      char buf[256];					\
      sprintf(buf, FMT "-> NULL", __VA_ARGS__);		\
      umem_set_status(ME, ERROR, buf);			\
      ERRRETURN;					\
    }							\
  } while (0)

/*
  Implementations of umemHostMemory methods.
*/

static uintptr_t umemHost_alloc_(umemVirtual * const me, size_t nbytes) {
  assert(me->type == umemHostDevice);
  uintptr_t adr = 0;
  if (nbytes != 0)
    HOST_CALL(me, adr = (uintptr_t)malloc(nbytes),
	      umemMemoryError, return 0,
	      "umemHost_alloc_: malloc(%ld)", nbytes
	      );
  return adr;
}

static void umemHost_free_(umemVirtual * const me, uintptr_t adr) {
  assert(me->type == umemHostDevice);
  free((void*)adr);
}

static void umemHost_set_(umemVirtual * const me, uintptr_t adr, int c, size_t nbytes) {
  assert(me->type == umemHostDevice);
  HOST_CALL(me, memset((void*)adr, c, nbytes), umemMemoryError, return,
	    "umemHost_set_: memset(&%lx, %d, %ld)", adr, c, nbytes);
}

static void umemHost_copy_to_(umemVirtual * const me, uintptr_t src_adr,
			      umemVirtual * const she, uintptr_t dest_adr,
			      size_t nbytes) {
  assert(me->type == umemHostDevice);
  if (she->type == umemHostDevice) {
    HOST_CALL(me, memcpy((void*)dest_adr, (void*)src_adr, nbytes), umemMemoryError, return,
	      "umemHost_copy_to_: memcpy(%lx, %lx, %ld)", dest_adr, src_adr, nbytes);
  } else
    umem_copy_from(she, dest_adr, me, src_adr, nbytes);
}

static void umemHost_copy_from_(umemVirtual * const me, uintptr_t dest_adr,
				umemVirtual * const she, uintptr_t src_adr,
				size_t nbytes) {
  assert(me->type == umemHostDevice);
  if (she->type == umemHostDevice)
    umemHost_copy_to_(she, src_adr, me, dest_adr, nbytes);
  else
    umem_copy_to(she, src_adr, me, dest_adr, nbytes);
}

/*
  umemHost constructor.
*/

void umemHost_ctor(umemHost * const me) {
  static struct umemVtbl const vtbl = {
    &umemHost_alloc_,
    &umemHost_free_,
    &umemHost_set_,
    &umemHost_copy_to_,
    &umemHost_copy_from_,
  };
  umemVirtual_ctor(&me->super);
  me->super.type = umemHostDevice;
  me->super.vptr = &vtbl;
}


/*
  umemVirtual copy methods that use host memory as an intermediate buffer.
 */
void umem_copy_to_via_host(void * const me, uintptr_t src_adr,
			   void * const she, uintptr_t dest_adr,
			   size_t nbytes) {
  umemHost host;
  umemHost_ctor(&host);
  if (umem_is_ok((void*)&host)) {
    uintptr_t host_adr = umem_alloc(&host, nbytes);
    if (umem_is_ok((void*)&host)) {
      umem_copy_to(me, src_adr, &host, host_adr, nbytes);
      if (umem_is_ok(me))
	umem_copy_from(she, dest_adr, &host, host_adr, nbytes);
      umem_free(&host, host_adr);
    }
  }
  umemHost_dtor(&host);
}

void umem_copy_from_via_host(void * const me, uintptr_t dest_adr,
			     void * const she, uintptr_t src_adr,
			     size_t nbytes) {
  umemHost host;
  umemHost_ctor(&host);
  if (umem_is_ok((void*)&host)) {
    uintptr_t host_adr = umem_alloc(&host, nbytes);
    if (umem_is_ok((void*)&host)) {
      umem_copy_to(she, src_adr, &host, host_adr, nbytes);
      if (umem_is_ok(she))
	umem_copy_from(me, dest_adr, &host, host_adr, nbytes);
      umem_free(&host, host_adr);
    }
  }
  umemHost_dtor(&host);
}