#include "aos/ipc_lib/multichannel_queue_racer.h"

#include "gtest/gtest.h"

namespace aos::ipc_lib::testing {
#if defined(AOS_IPC_LIB_LOCKLESS_QUEUE_HAS_ATOMIC_TIME_POINT)
TEST(MultiChannelQueueRacerTest, Race) {
  MultiChannelQueueRacer racer(/*num_threads=*/10, /*num_messages=*/1000000);
  racer.Run();
}
#elif defined(AOS_IPC_LIB_TEST_CAN_RELIABLY_TRIGGER_RACES)
// When we *don't* have the AOS_IPC_LIB_LOCKLESS_QUEUE_HAS_ATOMIC_TIME_POINT
// variable set we *should* be able to trigger the race condition in question.
TEST(MultiChannelQueueRacerDeathTest, Race) {
  MultiChannelQueueRacer racer(/*num_threads=*/10, /*num_messages=*/1000000);
  EXPECT_DEATH(racer.Run(), "Check failed.*recent_send_times.*< receive_time");
}
#endif  // AOS_IPC_LIB_LOCKLESS_QUEUE_HAS_ATOMIC_TIME_POINT
}  // namespace aos::ipc_lib::testing
