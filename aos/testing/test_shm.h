#ifndef AOS_TESTING_TEST_SHM_H_
#define AOS_TESTING_TEST_SHM_H_

#include <stddef.h>

namespace aos::testing {

// Allocates a block of memory which will be shared on a fork.
class SharedMemoryBlock {
 public:
  explicit SharedMemoryBlock(size_t size);
  ~SharedMemoryBlock();

  // Delete copy constructors
  SharedMemoryBlock(const SharedMemoryBlock &) = delete;
  SharedMemoryBlock(SharedMemoryBlock &&other) noexcept = delete;
  SharedMemoryBlock &operator=(const SharedMemoryBlock &) = delete;
  SharedMemoryBlock &operator=(SharedMemoryBlock &&other) noexcept = delete;

  void *get() const { return addr_; }
  size_t size() const { return size_; }

 private:
  void *addr_ = nullptr;
  size_t size_ = 0;
};

}  // namespace aos::testing

#endif  // AOS_TESTING_TEST_SHM_H_
