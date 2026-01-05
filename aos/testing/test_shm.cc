#include "aos/testing/test_shm.h"

#include <stddef.h>
#include <sys/mman.h>

#include "absl/log/absl_check.h"

namespace aos::testing {

// OSX and Linux have different names for the same thing.
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif

SharedMemoryBlock::SharedMemoryBlock(size_t size) : size_(size) {
  addr_ = mmap(nullptr, size_, PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  ABSL_PCHECK(addr_ != MAP_FAILED);
}

SharedMemoryBlock::~SharedMemoryBlock() {
  if (addr_ != nullptr && addr_ != MAP_FAILED) {
    munmap(addr_, size_);
  }
}

}  // namespace aos::testing
