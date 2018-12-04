#include <assert.h>
#include <string.h>
#include "umem.h"

/*
  umemVirtual virtual methods.
*/

static void umem_dtor_(umemVirtual  * const me) {
  assert(0); /* purely-virtual function should never be called */
}


static uintptr_t umem_alloc_(umemVirtual  * const me, size_t nbytes) {
  assert(0); /* purely-virtual function should never be called */
  return 0;
}


static uintptr_t umem_calloc_(umemVirtual  * const me, size_t nmemb, size_t size) {
  assert(0); /* purely-virtual function should never be called */
  return 0;
}


static void umem_free_(umemVirtual  * const me, uintptr_t adr) {
  assert(0); /* purely-virtual function should never be called */
}


static void umem_set_(umemVirtual * const me, uintptr_t adr, int c, size_t nbytes) {
  assert(0); /* purely-virtual function should never be called */
}


static void umem_copy_to_(umemVirtual * const me, uintptr_t src_adr,
			  umemVirtual * const she, uintptr_t dest_adr,
			  size_t nbytes) {
  assert(0); /* purely-virtual function should never be called */
}


static void umem_copy_from_(umemVirtual  * const me, uintptr_t src_adr,
			    umemVirtual  * const she, uintptr_t dest_adr,
			    size_t nbytes) {
  assert(0); /* purely-virtual function should never be called */
}


/*
  umemVirtual constructor.
*/
void umemVirtual_ctor(umemVirtual * const me, umemHost * host) {
  static struct umemVtbl const vtbl = {
    &umem_dtor_,
    &umem_alloc_,
    &umem_calloc_,
    &umem_free_,
    &umem_set_,
    &umem_copy_to_,
    &umem_copy_from_,
  };
  me->vptr = &vtbl;
  me->type = umemVirtualDevice;
  me->status.type = umemOK;
  // message is owned by umemVirtual instance. So, use only
  // umem_set_status or umem_clear_status to change it.
  me->status.message = NULL;
  me->host = (void*)host;
}


/*
  umemVirtual destructor.
*/
void umemVirtual_dtor(umemVirtual * const me) {
  if (me->status.message != NULL) {
    free(me->status.message);
    me->status.message = NULL;
  }
  me->status.type = umemOK;
  if (me->host != NULL) {
    umem_dtor(me->host);
    me->host = NULL;
  }
}


uintptr_t umemVirtual_calloc(umemVirtual * const me, size_t nmemb, size_t size) {
  uintptr_t adr = 0;
  if (size != 0) {
    size_t nbytes = nmemb * size; // TODO: check overflow
    adr = umem_alloc(me, nbytes);
    if (umem_is_ok(me))
      umem_set(me, adr, 0, nbytes);
  }
  return adr;
}


uintptr_t umem_aligned_calloc(umemVirtual * const me, size_t alignment, size_t size) {
  uintptr_t adr = 0;
  if (size == 0) return adr;
  /*
  HOST_CALL(me, !umem_ispowerof2(alignment), umemValueError, return 0,
            "umemVirtual_aligned_alloc: alignment %zu must be power of 2",
            alignment);
  HOST_CALL(me, size % alignment, umemValueError, return 0,
            "umemVirtual_aligned_alloc: size %zu must be multiple of alignment %zu",
            size, alignment);
  */
  size_t extra = (alignment - 1) + sizeof(uintptr_t);
  size_t req = extra + size;
  adr = umem_calloc(me, req, 1);
  if (adr==0) return adr;  // this must be error
  uintptr_t aligned = adr + extra;
  aligned = aligned - (aligned % alignment);

  umem_copy_to(me->host, (uintptr_t)&adr, me, aligned-sizeof(uintptr_t), sizeof(uintptr_t)); // todo: check status
  //*((uintptr_t *)aligned - 1) = adr;
  return aligned;
}


void umem_aligned_free(void * const me, uintptr_t aligned_adr) {
  umemVirtual * const me_ = me;
  if (aligned_adr == 0) return;
  uintptr_t adr = 0;
  umem_copy_from(me_->host, (uintptr_t)&adr, me, aligned_adr-sizeof(uintptr_t), sizeof(uintptr_t)); // todo: check status
  umem_free(me, adr);
}

/*
  Status handling utility functions.
*/
void umem_set_status(void * const me,
		     umemStatusType type, const char * message) {
  umemVirtual * const me_ = me;
  if (message == NULL) {
    if (me_->status.message != NULL)
      free(me_->status.message);
    me_->status.message = NULL;
  } else {
    if (me_->status.message == NULL) {
      me_->status.message = strdup(message);
    } else {
      // append message
      char buf[256];
      buf[0] = 0;
      if (me_->status.type != type) {
	snprintf(buf, sizeof(buf), "\nstatus %s changed to %s",
		 umem_get_status_name(me_->status.type),
		 umem_get_status_name(type));
      }
      size_t l1 = strlen(me_->status.message);
      size_t l2 = strlen(buf);
      size_t l3 = strlen(message);
      me_->status.message = realloc(me_->status.message,
				    l1 + l2 + l3 + 2);
      memcpy(me_->status.message + l1, buf, l2);
      memcpy(me_->status.message + l1 + l2, "\n", 1);
      memcpy(me_->status.message + l1 + l2 + 1, message, l3);
      me_->status.message[l1+l2+l3+1] = '\0';
    }
  }
  me_->status.type = type;
}


void umem_clear_status(void * const me) {
  umemVirtual * const me_ = me;
  if (me_->status.message != NULL) {
    free(me_->status.message);
    me_->status.message = NULL;
  }
  me_->status.type = umemOK;
}

/*
  Utility functions
*/

const char* umem_get_device_name(umemDeviceType type) {
  static const char virtual_device_name[] = "Virtual";
  static const char host_device_name[] = "Host";
  static const char file_device_name[] = "FILE";
  static const char cuda_device_name[] = "CUDA";
  static const char mmap_device_name[] = "MMAP"; // NOT IMPLEMENTED
  switch(type) {
  case umemVirtualDevice: return virtual_device_name;
  case umemHostDevice: return host_device_name;
  case umemFileDevice: return file_device_name;
  case umemCudaDevice: return cuda_device_name;
  }
  return NULL;
}


const char* umem_get_status_name(umemStatusType type) {
  static const char ok_name[] = "OK";
  static const char memory_error_name[] = "MemoryError";
  static const char runtime_error_name[] = "RuntimeError";
  static const char io_error_name[] = "IOError";
  static const char notimpl_error_name[] = "NotImplementedError";
  static const char assert_error_name[] = "AssertError";
  static const char value_error_name[] = "ValueError";
  static const char type_error_name[] = "TypeError";
  switch (type) {
  case umemOK: return ok_name;
  case umemMemoryError: return memory_error_name;
  case umemRuntimeError: return runtime_error_name;
  case umemIOError: return io_error_name;
  case umemNotImplementedError: return notimpl_error_name;
  case umemAssertError: return assert_error_name;
  case umemValueError: return value_error_name;
  case umemTypeError: return type_error_name;
  }
  return NULL;
}
