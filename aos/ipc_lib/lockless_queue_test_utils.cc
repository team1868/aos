#include "aos/ipc_lib/lockless_queue_test_utils.h"

#include "aos/ipc_lib/signalfd.h"
namespace aos::ipc_lib::testing {

LocklessQueueTest::PinForTest::PinForTest() {
  cpu_set_t cpus = GetCurrentThreadAffinity();
  old_ = cpus;
  int number_found = 0;
  for (int i = 0; i < CPU_SETSIZE; ++i) {
    if (CPU_ISSET(i, &cpus)) {
      if (number_found < 2) {
        ++number_found;
      } else {
        CPU_CLR(i, &cpus);
      }
    }
  }
  SetCurrentThreadAffinity(cpus);
}

LocklessQueueTest::LocklessQueueTest() {
  config_.num_watchers = 10;
  config_.num_senders = 100;
  config_.num_pinners = 5;
  config_.queue_size = 10000;
  // Exercise the alignment code.  This would throw off alignment.
  config_.message_data_size = 101;

  // Since our backing store is an array of uint64_t for alignment purposes,
  // normalize by the size.
  memory_.resize(LocklessQueueMemorySize(config_) / sizeof(uint64_t));

  Reset();
}

void LocklessQueueTest::RunUntilWakeup(Event *ready, int priority) {
  internal::EPoll epoll;
  SignalFd signalfd({kWakeupSignal});

  epoll.OnReadable(signalfd.fd(), [&signalfd, &epoll]() {
    signalfd_siginfo result = signalfd.Read();

    fprintf(stderr, "Got signal: %d\n", result.ssi_signo);
    epoll.Quit();
  });

  {
    // Register to be woken up *after* the signalfd is catching the signals.
    LocklessQueueWatcher watcher =
        LocklessQueueWatcher::Make(queue(), priority).value();

    // And signal we are now ready.
    ready->Set();

    epoll.Run();

    // Cleanup, ensuring the watcher is destroyed before the signalfd.
  }
  epoll.DeleteFd(signalfd.fd());
}
}  // namespace aos::ipc_lib::testing
