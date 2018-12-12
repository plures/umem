# UMEM
Unifying MEmory Management library for connecting different memory devices and interfaces

[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/kcedl490a7fbqvoo/branch/master?svg=true)](https://ci.appveyor.com/project/pearu/umem/branch/master)
[![CircleCI Build Status](https://circleci.com/gh/plures/umem/tree/master.svg?style=svg)](https://circleci.com/gh/plures/umem/tree/master)
[![Travis Build Status](https://travis-ci.org/plures/umem.svg?branch=master)](https://travis-ci.org/plures/umem)

## Introduction

Memory management is an integral part of any software
library. Libraries such as [XND](https://xnd.io) (efficient array
representations), [Apache Arrow](https://arrow.apache.org/) (in-memory
data storage), and others that implement data storage formats for
various applications, need to support different memory locations where
the data can be processed. For instance, the most widely used memory
location is CPU RAM but with the advances in making other devices such
as GPU available to data processing applications, the data storage
libraries need to deal with diverse APIs that different device drivers
libraries have. There exists a number of projects such as
[OpenCL](https://www.khronos.org/opencl/),
[HSA](https://en.wikipedia.org/wiki/Heterogeneous_System_Architecture),
etc.  that aim at providing unifying interfaces to the heterogeneous
set of devices. But these projects are very large and performance-wise
can be sub-optimal (ref CUDA vs OpenCL).

The UMEM project aims at providing a portable and easy-to-use
interface to memory management tasks of most popular devices such as
CPU, GPU, as well as of different memory management interfaces (files,
mmap, memory pools, etc).

The core part of UMEM is written in C for efficency but using OOP
design to be easily extensible. UMEM provides also C++ API that usage
is highly recommended for C++ programs.

Plans are interfacing UMEM to high-level scripting languages such as
Python, etc.

## UMEM memory model

The fundamental idea behind UMEM design is that the data location in
any storage device or interface can be represented as an integer
valued address within the context of the device or interface. For
instance, the data in host RAM can be referred to via C pointer value
which can be cast to an integer; the data in GPU device memory can be
referred to via C pointer value as well; the data in a file can also
be addressed using the position returned by `ftell` function. And so
on.

In C core part, UMEM uses `uintptr_t` type (defined in stdint.h) for
addressing data in any supported storage device.  UMEM provides
device-independent API and efficient implementations for copying data
between different devices.  For instance, one can use a single UMEM
API function to copy data from GPU device memory, say, to a file on
the disk drive.

In C++ API, UMEM provides `Address` class that represents a memory
address with a device context. `Address` defines convinient casting
and pointer arithmetics operators as well as methods for copying and
syncronizing data between different devices.

## Example in C++

```c++

#include "umem.h"

int main() {
  // Define and construct the context objects
  umem::Host host;
  umem::Cuda cuda(0); // 0 is GPU device number
  umem::File file("/path/to/my/file.dat", "w+b")
  
  // Allocate memory for an array of doubles in host
  size_t nbytes = 64 * sizeof(double);
  umem::Address host_adr = host.alloc(size);

  // Zero the data
  host_adr.set(0, nbytes);
  // or initialize it as a `range(64)`:
  for (int i=0; i<64; ++i) ((double*)host_adr)[i] = i;

  // Allocate memory in GPU device
  umem::Address cuda_adr = cuda.alloc(nbytes/2);

  // Copy a slice of host data (`range(64)[10:42]`) to GPU device:
  (host_adr+10*sizeof(double)).copy_to(cuda_adr, nbytes/2);

  // Open file for writing
  umem::Address file_adr = file.alloc(nbytes/2);

  // Copy data from GPU device to file:
  file_adr.copy_from(cuda_adr, nbytes/2);
}
```

## Example in C

```c

#include "umem.h"

int main() {
  // Define context objects 
  umemHost host;
  umemCuda cuda;
  umemFile file;

  // Construct the context objects
  umemHost_ctor(&host);
  umemCuda_ctor(&cuda, 0); // 0 is GPU device number
  umemFile_ctor(&file, "/path/to/my/file.dat", "w+b");

  // Allocate memory for an array of doubles in host
  size_t nbytes = 64 * sizeof(double);
  uintptr_t host_adr = umem_alloc(&host, nbytes);

  // Zero the data
  umem_set(&host, host_adr, 0, nbytes);
  // or initialize it as a `range(64)`:
  for (int i=0; i<64; ++i) ((double*)host_adr)[i] = i;

  // Allocate memory in GPU device
  uintptr_t cuda_adr = umem_alloc(&cuda, nbytes/2);

  // Copy a slice of host data (`range(64)[10:42]`) to GPU device:
  umem_copy_to(&host, host_adr+10*sizeof(double), &cuda, cuda_adr, nbytes/2);

  // Open file for writing
  uintptr_t file_adr = umem_alloc(&file, nbytes/2);

  // Copy data from GPU device to file:
  umem_copy_from(&file, file_adr, &cuda, cuda_adr, nbytes/2);

  // Free memory
  umem_free(&host, host_adr);
  umem_free(&cuda, cuda_adr);
  umem_free(&file, file_adr); // this closes the file

  // Deconstruct the context objects
  umem_dtor(&host);
  umem_dtor(&cuda);
  umem_dtor(&file);
}
```

## Features

### Currently supported devices

```
Host - using malloc/free/memcpy/memset
Cuda - using cudaMalloc/cudaFree/cudaMemcpy/cudaMemset
File - using fopen/fclose/fread,fwrite
```

Public API:
```
Context types:
  umemHost
  umemCuda
  umemFile
Constructors:
  umemHost_ctor(&this)
  umemCuda_ctor(&this, device)
  umemFile_ctor(&this, filename, mode)
Destructors:
  umem_dtor(&this)
```
Here and in the following, `me` is object of type `umemHost` or
`umemCuda` or `umemFile`.

### Planned device/interface support

```
MMap
Cu              - using cuMemAlloc/cuMemFree
CudaManaged
CudaHost
ArrowBuffer
RMM             - see cudf
```

### Memory management

Public API:
```
umem_alloc(&this, nbytes) -> adr
umem_calloc(&this, nmemb, size) -> adr
umem_free(&this, adr)
umem_aligned_alloc(&this, alignment, nbytes) -> aligned_adr
umem_aligned_origin(&this, aligned_adr) -> adr
umem_aligned_free(&this, aligned_adr)
umem_set(&this, adr, c, nbytes)
```

### Connecting devices

Public API:
```
umem_is_same_device(&this, &that) -> bool
umem_connect(&src, src_adr, n, &dest, dest_alignment) -> dest_adr
umem_sync_to(&src, src_adr, &dest, dest_adr, n)
umem_sync_from(&src, src_adr, &dest, dest_adr, n)
umem_disconnect(&src, src_adr, &dest, dest_adr, dest_alignment)

umem_copy_to(&this, this_adr, &that, that_adr, nbytes)
umem_copy_from(&this, this_adr, &that, that_adr, nbytes)
```
When `src` and `dest` are the same device, `umem_connect`
returns `src_adr`, otherwise, the address of newly allocated `dest`
memory is returned. Use `umem_sync_to/from` to keep device memories in
sync.  `umem_disconnect` frees memory allocated by `umem_connect`. Use
`dest_alignment=0` for default alignment.

Explicit copy implementations:
```
host -> host
host <- host
cuda N -> host
cuda N -> cuda M
cuda N <- host
file -> host
file <- host
```

Derived copy implementations:
```
file -> host -> cuda
file <- host <- cuda
```


### Error handling

umem context instances have `status` attribute that is used for passing around messages.

Public API:
```
umem_get_status(&this) -> <status type>
umem_get_message(&this) -> <status message>
umem_is_ok(&this) -> <status is ok>
umem_set_status(&this, type, message)
umem_clear_status(&this)
```

## Prerequisites

```
conda install cmake make gcc_linux-64 gxx_linux-64 valgrind -c conda-forge
```

## Building and testing UMEM

```
git clone https://github.com/plures/umem.git
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../umem/c
make
make test                                     # runs unittests
ctest -D ExperimentalMemCheck -E test_cuda    # runs valgrind
```

## Extending UMEM

To add support for a new memory device or interface, one must:
```
update enums in umem.h
implement constructor/desrtructor functions
implement alloc, free, set methods
implement copy_to and copy_from methods, supporting the host source and host destination is mandatory.
update docs
```
