#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/absl_log.h"
#include "absl/log/initialize.h"

#include "aos/events/shm_event_loop.h"
#include "aos/events/simulated_event_loop.h"
#include "aos/init.h"
#include "aos/sanitizers.h"
#include "aos/testing/path.h"

// Test flags to control behaviour.
ABSL_FLAG(std::string, event_loop_type, "shm",
          "Type of event loop to use. Options: shm, simulated.");
ABSL_FLAG(std::string, test_mode, "basic",
          "Test mode: basic, configure_thread, ignore_thread, invalid_thread, "
          "double_ignore, configure_ignored, thread_crash");
ABSL_FLAG(std::string, thread_name, "test_thread",
          "Name of the thread to configure or ignore");
ABSL_FLAG(
    bool, lock_to_main_thread, true,
    "Whether to lock the event loop to the main thread. Only relevant for "
    "--event_loop_type=shm.");
ABSL_FLAG(bool, print_in_rt_thread, false,
          "Whether to print in the real-time worker thread.");
ABSL_FLAG(int32_t, delay_before_shutdown_ms, 100,
          "Delay in milliseconds before shutting down the event loop.");
ABSL_FLAG(
    std::string, on_run_direction, "acquire",
    "Direction of the semaphore in OnRun. One of \"acquire\", \"release\".");

namespace {

// Create a minimal configuration for testing.
aos::FlatbufferDetachedBuffer<aos::Configuration> CreateTestConfiguration() {
  return aos::configuration::ReadConfig(aos::testing::ArtifactPath(
      "aos/events/event_loop_thread_tester_config.json"));
}

class TestHelper {
 public:
  TestHelper()
      : config_(CreateTestConfiguration()),
        thread_name_(absl::GetFlag(FLAGS_thread_name)) {
    // Instantiate the correct event loop type.
    if (absl::GetFlag(FLAGS_event_loop_type) == "simulated") {
      simulated_event_loop_container_ =
          std::make_unique<SimulatedEventLoopContainer>(&config_.message());
      event_loop_ = simulated_event_loop_container_->event_loop.get();
      exit_handle_ = simulated_event_loop_container_->factory.MakeExitHandle();
    } else {
      shm_event_loop_ = std::make_unique<aos::ShmEventLoop>(&config_.message());
      event_loop_ = shm_event_loop_.get();
      exit_handle_ = shm_event_loop_->MakeExitHandle();

      // Lock the event loop to the main thread if requested.
      if (absl::GetFlag(FLAGS_lock_to_main_thread)) {
        shm_event_loop_->LockToThread();
      }
    }

    ABSL_LOG(INFO) << "Application name: " << event_loop_->name();
    event_loop_->SkipAosLog();
    event_loop_->SkipTimingReport();

    // Add a timer to exit the event loop after a delay as per
    // --delay_before_shutdown_ms. We want to quit pretty quickly by default.
    // But the user can customize this.
    timer_ = event_loop_->AddTimer([this] { exit_handle_->Exit(); });
    event_loop_->OnRun([this] {
      timer_->Schedule(event_loop_->monotonic_now() +
                       std::chrono::milliseconds(
                           absl::GetFlag(FLAGS_delay_before_shutdown_ms)));
    });
  }
  virtual ~TestHelper() { ABSL_LOG(INFO) << "Test finished without error."; }

  // Runs the specific test.
  virtual void RunTest() = 0;

 protected:
  struct SimulatedEventLoopContainer {
    SimulatedEventLoopContainer(const aos::Configuration *configuration)
        : factory(configuration),
          event_loop(factory.MakeEventLoop("event_loop_thread_tester")) {}

    aos::SimulatedEventLoopFactory factory;
    std::unique_ptr<aos::EventLoop> event_loop;
  };

  // Runs the corresponding event loop.
  void RunEventLoop() {
    if (simulated_event_loop_container_) {
      simulated_event_loop_container_->factory.Run();
    } else {
      shm_event_loop_->Run();
    }
  }

  aos::FlatbufferDetachedBuffer<aos::Configuration> config_;

  // We will configure one of these two event loops.
  std::unique_ptr<SimulatedEventLoopContainer> simulated_event_loop_container_;
  std::unique_ptr<aos::ShmEventLoop> shm_event_loop_;
  std::unique_ptr<aos::ExitHandle> exit_handle_;
  aos::EventLoop *event_loop_;
  aos::TimerHandler *timer_;
  std::string thread_name_;
};

// Validates that simple instantiation (and no Run() call) doesn't block
// anything.
class TestBasic : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Running basic ShmEventLoop test...";
  }
};

class TestThreadConfigurationTimeout : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing thread configuration timeout...";
    ABSL_LOG(INFO) << "Configuring thread: " << thread_name_;

    // We intentionally don't configure the thread here so it triggers a
    // timeout.
    RunEventLoop();
  }
};

// Helps validate behaviour when configuring a thread from the main thread.
class TestConfigureThreadFromMain : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO)
        << "Testing ConfigureThreadAndWaitForRun from main thread...";
    ABSL_LOG(INFO) << "Configuring thread: " << thread_name_;

    std::unique_ptr<aos::ThreadHandle> handle =
        event_loop_->ConfigureThreadAndWaitForRun(thread_name_);
  }
};

// Helps validate behaviour when configuring a thread.  Can be used with threads
// from the config or non-existent threads for different behaviours.
class TestConfigureThread : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing ConfigureThreadAndWaitForRun...";
    ABSL_LOG(INFO) << "Configuring thread: " << thread_name_;

    std::thread test_thread(
        [this]() {
          {
            std::unique_ptr<aos::ThreadHandle> handle =
                event_loop_->ConfigureThreadAndWaitForRun(thread_name_);

            if (absl::GetFlag(FLAGS_print_in_rt_thread)) {
#if defined(AOS_SANITIZE_ADDRESS) || defined(AOS_SANITIZE_MEMORY) || \
    defined(AOS_SANITIZE_THREAD)
              // When using sanitizers, we cannot intercept malloc calls. So we
              // trigger an explicit crash here that would normally happen.
              ABSL_LOG(FATAL)
                  << "Cannot trigger \"RAW: Malloced \". Crashing anyway...";
#else
              ABSL_LOG(INFO) << "Thread configured, now crashing...";
#endif
            }

            // Do some fake work in the thread.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
          }

          ABSL_LOG(INFO) << "Thread work completed";
        });

    RunEventLoop();
    test_thread.join();
  }
};

// Triggers a crash by having two different threads claim to be the same thread
// during configuration.
class TestConfigureThreadTwice : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing ConfigureThreadAndWaitForRun...";
    ABSL_LOG(INFO) << "Configuring 2 threads: " << thread_name_;

    std::thread test_thread1([this]() {
      std::unique_ptr<aos::ThreadHandle> handle =
          event_loop_->ConfigureThreadAndWaitForRun(thread_name_);
    });
    std::thread test_thread2([this]() {
      std::unique_ptr<aos::ThreadHandle> handle =
          event_loop_->ConfigureThreadAndWaitForRun(thread_name_);
    });

    RunEventLoop();
    test_thread1.join();
    test_thread2.join();
  }
};

// Helps validate the behaviour of ignoring a thread. Can be used with threads
// from the config or non-existent threads for different behaviours.
class TestIgnoreThread : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing IgnoreThread...";
    ABSL_LOG(INFO) << "Ignoring thread: " << thread_name_;
    event_loop_->IgnoreThread(thread_name_);
    RunEventLoop();
  }
};

// Triggers a crash by ignoring the same thread twice.
class TestDoubleIgnore : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing double IgnoreThread...";

    ABSL_LOG(INFO) << "First ignore of thread: " << thread_name_;
    event_loop_->IgnoreThread(thread_name_);
    ABSL_LOG(INFO) << "Second ignore of same thread: " << thread_name_;
    event_loop_->IgnoreThread(thread_name_);
  }
};

// Triggers a crash by trying to configure an ignored thread.
class TestConfigureIgnoredThread : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO)
        << "Testing ConfigureThreadAndWaitForRun on ignored thread...";

    ABSL_LOG(INFO) << "Ignoring thread: " << thread_name_;
    event_loop_->IgnoreThread(thread_name_);

    ABSL_LOG(INFO) << "Attempting to configure ignored thread: "
                   << thread_name_;
    std::thread test_thread([this]() {
      auto handle = event_loop_->ConfigureThreadAndWaitForRun(thread_name_);
    });

    RunEventLoop();
    test_thread.join();
  }
};

// Helps validate what happens when you ignore an already-configured thread.
class TestIgnoreConfiguredThread : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing IgnoreThread on configured thread...";

    // We want to validate the behaviour when you ignore an already-configured
    // thread. To do this, we will configure the thread in a separate thread and
    // then ignore it. This is a bit tricky because we cannot 100% guarantee
    // that the thread is configured before we ignore it. To _attempt_ to do
    // this, we use a semaphore for synchronization and then sleep in the main
    // thread a bit in the hopes that the configured thread has started
    // configuring itself.
    std::binary_semaphore semaphore(0);

    ABSL_LOG(INFO) << "Configuring thread: " << thread_name_;
    std::thread test_thread([this, &semaphore]() {
      semaphore.release();
      auto handle = event_loop_->ConfigureThreadAndWaitForRun(thread_name_);
    });

    // Wait for the thread to get to the ConfigureThreadAndWaitForRun call.
    semaphore.acquire();

    // Sleep a little bit in the hopes that the thread will actually start
    // configuring itself in the meantime.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // This should now ideally cause a crash.
    ABSL_LOG(INFO) << "Ignoring configured thread: " << thread_name_;
    event_loop_->IgnoreThread(thread_name_);
  }
};

// Helps validate behaviour when ignoring a thread from a place other than the
// main thread.
class TestIgnoreThreadFromThread : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing IgnoreThread from a non-main thread...";
    ABSL_LOG(INFO) << "Configuring thread: " << thread_name_;

    std::thread thread([this] { event_loop_->IgnoreThread(thread_name_); });
    thread.join();
  }
};

// Validates that we can successfully call Run() multiple times in simulation.
// This is a pattern we use very frequently so we need to make sure it works.
// We must only deal with thread configuration and startup synchronization the
// first time we call Run().
class TestMultipleRunsWorksInSimulation : public TestHelper {
 public:
  void RunTest() override {
    ABSL_CHECK(simulated_event_loop_container_)
        << "This test is only supported in simulation.";

    ABSL_LOG(INFO)
        << "Testing that we can run the event loop multiple times...";

    ABSL_LOG(INFO) << "Configuring thread: " << thread_name_;
    std::thread test_thread([this]() {
      {
        auto handle = event_loop_->ConfigureThreadAndWaitForRun(thread_name_);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      ABSL_LOG(INFO) << "Thread work completed.";
    });

    // We want to make sure that we only block the Run() once. Subsequent calls
    // should not block.
    ABSL_LOG(INFO) << "Triggering Run.";
    RunEventLoop();
    ABSL_LOG(INFO) << "Triggering Run.";
    RunEventLoop();
    ABSL_LOG(INFO) << "Triggering Run.";
    RunEventLoop();

    test_thread.join();
  }
};

// Validates that OnRun and ConfigureThreadAndWaitForRun unblock at the same
// time. We do this by using a binary semaphore to release/acquire in both
// directions.
class TestOnRunUnblocksWithThread : public TestHelper {
 public:
  void RunTest() override {
    ABSL_LOG(INFO) << "Testing OnRun...";

    std::binary_semaphore semaphore(0);

    event_loop_->OnRun([&semaphore]() {
      // Perform the action specified by the flag.
      if (absl::GetFlag(FLAGS_on_run_direction) == "acquire") {
        semaphore.acquire();
      } else {
        semaphore.release();
      }
    });

    ABSL_LOG(INFO) << "Configuring thread: " << thread_name_;
    std::thread test_thread([this, &semaphore]() {
      {
        auto handle = event_loop_->ConfigureThreadAndWaitForRun(thread_name_);
        // Do the opposite of the OnRun function here.
        if (absl::GetFlag(FLAGS_on_run_direction) == "acquire") {
          semaphore.release();
        } else {
          semaphore.acquire();
        }
      }
      ABSL_LOG(INFO) << "Thread work completed.";
    });

    RunEventLoop();

    test_thread.join();
  }
};

}  // namespace

int main(int argc, char *argv[]) {
  // Initialize AOS
  aos::InitGoogle(&argc, &argv);

  const std::string &test_mode = absl::GetFlag(FLAGS_test_mode);
  std::unique_ptr<TestHelper> test;

  if (test_mode == "basic") {
    test = std::make_unique<TestBasic>();
  } else if (test_mode == "thread_configuration_timeout") {
    test = std::make_unique<TestThreadConfigurationTimeout>();
  } else if (test_mode == "configure_thread_from_main") {
    test = std::make_unique<TestConfigureThreadFromMain>();
  } else if (test_mode == "configure_thread") {
    test = std::make_unique<TestConfigureThread>();
  } else if (test_mode == "configure_thread_twice") {
    test = std::make_unique<TestConfigureThreadTwice>();
  } else if (test_mode == "ignore_thread") {
    test = std::make_unique<TestIgnoreThread>();
  } else if (test_mode == "double_ignore") {
    test = std::make_unique<TestDoubleIgnore>();
  } else if (test_mode == "configure_ignored") {
    test = std::make_unique<TestConfigureIgnoredThread>();
  } else if (test_mode == "ignore_configured") {
    test = std::make_unique<TestIgnoreConfiguredThread>();
  } else if (test_mode == "ignore_thread_from_non_main_thread") {
    test = std::make_unique<TestIgnoreThreadFromThread>();
  } else if (test_mode == "multiple_runs_works_in_simulation") {
    test = std::make_unique<TestMultipleRunsWorksInSimulation>();
  } else if (test_mode == "on_run_unblocks_with_thread") {
    test = std::make_unique<TestOnRunUnblocksWithThread>();
  } else {
    ABSL_LOG(FATAL) << "Unknown test mode: " << test_mode;
  }

  test->RunTest();

  return 0;
}
