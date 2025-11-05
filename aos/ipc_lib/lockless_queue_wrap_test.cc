#include <cinttypes>

#include "aos/ipc_lib/lockless_queue_test_utils.h"
#include "aos/ipc_lib/queue_racer.h"

namespace aos::ipc_lib::testing {

namespace chrono = ::std::chrono;
// Send enough messages to wrap the 32 bit send counter.
TEST_F(LocklessQueueTest, WrappedSend) {
  PinForTest pin_cpu;
  uint64_t kNumMessages = 0x100010000ul;
  QueueRacer racer(queue(), 1, kNumMessages);

  const monotonic_clock::time_point start_time = monotonic_clock::now();
  EXPECT_NO_FATAL_FAILURE(racer.RunIteration(false, 0, false, true));
  const monotonic_clock::time_point monotonic_now = monotonic_clock::now();
  double elapsed_seconds = chrono::duration_cast<chrono::duration<double>>(
                               monotonic_now - start_time)
                               .count();
  printf("Took %f seconds to write %" PRIu64 " messages, %f messages/s\n",
         elapsed_seconds, kNumMessages,
         static_cast<double>(kNumMessages) / elapsed_seconds);
}
}  // namespace aos::ipc_lib::testing
