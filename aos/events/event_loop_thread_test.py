"""Validates EventLoop thread functionality.

This test invokes a helper binary to validate the behaviour of the EventLoop
when we ask it to configure and/or ignore the threads in its AOS configuration.

This is a separate set of tests from the usual gtest infrastructure primarily
because death tests run the code under test under a different PID. This means
that our PID comparison logic doesn't work as expected. There are some
functions that need to be called from the main thread and others that
specifically need to _not_ be called from the main thread.
"""

import subprocess
import os
import sys
import unittest
from pathlib import Path

from python.runfiles import runfiles

RUNFILES = runfiles.Create()

TESTER_BINARY = RUNFILES.Rlocation("aos/aos/events/event_loop_thread_tester")

# The test needs to pass this in via "env" in the BUILD file. Either "shm" or
# "simulated".
EVENT_LOOP_TYPE = os.environ["EVENT_LOOP_TYPE"]


def run_underlying_tester_binary(*args: list[str],
                                 check: bool = True,
                                 **kwargs):
    """Run the test binary with the runfiles environment passed in.

    Args:
        *args: Arguments to pass to subprocess.run.
        check: Whether to check the return code.
        **kwargs: Keyword arguments to pass to subprocess.run.

    Returns:
        subprocess.CompletedProcess result.
    """
    env = os.environ.copy()
    env.update(RUNFILES.EnvVars())
    return subprocess.run(*args, check=check, env=env, **kwargs)


def run_test_binary(*,
                    test_mode: str,
                    thread_name: str,
                    expect_error: bool,
                    lock_to_main_thread: bool = True,
                    print_in_rt_thread: bool = False,
                    on_run_direction: str = "acquire",
                    delay_before_shutdown_ms: int = 100,
                    thread_configuration_timeout_seconds: int = 20):
    """Run the test binary with specified parameters.

    Args:
        test_mode: Test mode to run.
        thread_name: Name of the thread to configure or ignore.
        expect_error: Whether to expect a non-zero return code.
        lock_to_main_thread: Whether to lock the event loop to the main
            thread.
        print_in_rt_thread: Whether to print in the real-time worker thread.
        delay_before_shutdown_ms: Delay in milliseconds before shutting down
            the event loop.

    Returns:
        subprocess.CompletedProcess result.
    """
    cmd = [
        TESTER_BINARY,
        f"--event_loop_type={EVENT_LOOP_TYPE}",
        f"--test_mode={test_mode}",
        f"--thread_name={thread_name}",
        f"--lock_to_main_thread={lock_to_main_thread}",
        f"--print_in_rt_thread={print_in_rt_thread}",
        f"--on_run_direction={on_run_direction}",
        f"--delay_before_shutdown_ms={delay_before_shutdown_ms}",
        f"--thread_configuration_timeout_seconds={thread_configuration_timeout_seconds}",
    ]

    result = run_underlying_tester_binary(cmd,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.STDOUT,
                                          text=True,
                                          check=False)

    print(f"Command: {' '.join(cmd)}")
    print(f"Return code: {result.returncode}")
    print(f"Stdout: {result.stdout}")

    if expect_error:
        assert result.returncode != 0
    else:
        assert result.returncode == 0
        assert "Test finished without error." in result.stdout

    return result


class TestEventLoopThreads(unittest.TestCase):

    def test_basic_functionality(self):
        """Test basic EventLoop creation and functionality."""
        result = run_test_binary(
            test_mode="basic",
            thread_name="worker1",
            expect_error=False,
        )

    def test_thread_configuration_timeout(self):
        """Test that thread configuration times out."""
        result = run_test_binary(
            test_mode="thread_configuration_timeout",
            thread_name="worker1",
            expect_error=True,
            thread_configuration_timeout_seconds=1,
        )

        self.assertIn("Configuring thread: worker1", result.stdout)
        self.assertIn("Not all threads started within 1s.", result.stdout)

    @unittest.skipIf(
        EVENT_LOOP_TYPE == "simulated",
        "Simulated event loop are always locked to the main thread.")
    def test_configure_thread_from_main_without_locking(self):
        """Test that thread configuration from the main thread causes a crash."""
        result = run_test_binary(
            test_mode="configure_thread_from_main",
            thread_name="test_thread",
            expect_error=True,
            lock_to_main_thread=False,
        )

        self.assertIn("Configuring thread: test_thread", result.stdout)
        self.assertIn("Call LockToThread() before constructing any threads.",
                      result.stdout)

    def test_configure_thread_from_main_with_locking(self):
        """Test that thread configuration from the main thread causes a crash."""
        result = run_test_binary(
            test_mode="configure_thread_from_main",
            thread_name="test_thread",
            expect_error=True,
            lock_to_main_thread=True,
        )

        self.assertIn("Configuring thread: test_thread", result.stdout)
        self.assertIn("Do not call this function from the main thread.",
                      result.stdout)

    def test_configure_thread_that_doesnt_exist(self):
        """Test thread configuration with the wrong name."""
        result = run_test_binary(
            test_mode="configure_thread",
            thread_name="non_existent_thread",
            expect_error=True,
        )

        self.assertIn("Configuring thread: non_existent_thread", result.stdout)
        self.assertIn(
            "No thread with name \"non_existent_thread\" found in the AOS configuration",
            result.stdout)

    def test_configure_thread_that_does_exist(self):
        """Test thread configuration with the correct name."""
        result = run_test_binary(
            test_mode="configure_thread",
            thread_name="test_thread",
            expect_error=False,
        )

        self.assertIn("Configuring thread: test_thread", result.stdout)
        self.assertIn("Waiting 20s for 1 thread to start", result.stdout)
        self.assertIn(
            "Thread test_thread waiting for the event loop to start running.",
            result.stdout)
        self.assertIn("Threads have started. Continuing.", result.stdout)
        self.assertIn("Thread work complete", result.stdout)

    def test_configure_same_thread_twice(self):
        """Test that two threads claiming the same name causes a crash."""
        result = run_test_binary(
            test_mode="configure_thread_twice",
            thread_name="test_thread",
            expect_error=True,
        )

        self.assertIn("Configuring 2 threads: test_thread", result.stdout)
        self.assertIn(
            "Another thread has already been configured under the name test_thread.",
            result.stdout)

    def test_run_thread_that_prints_in_realtime(self):
        """Test that an RT thread printing causes a crash."""
        result = run_test_binary(
            test_mode="configure_thread",
            thread_name="test_thread",
            expect_error=True,
            print_in_rt_thread=True,
            # Make sure the main thread doesn't shut down before the RT
            # thread has a chance to crash.
            delay_before_shutdown_ms=100000,
        )

        self.assertIn("Configuring thread: test_thread", result.stdout)
        self.assertIn("RAW: Malloced ", result.stdout)

    def test_ignore_thread_success(self):
        """Test successful thread ignoring."""
        result = run_test_binary(
            test_mode="ignore_thread",
            thread_name="test_thread",
            expect_error=False,
        )

        self.assertIn("Ignoring thread: test_thread", result.stdout)

    def test_ignore_invalid_thread(self):
        """Test that ignoring an invalid thread name fails appropriately."""
        result = run_test_binary(
            test_mode="ignore_thread",
            thread_name="nonexistent_thread",
            expect_error=True,
        )

        self.assertIn("Ignoring thread: nonexistent_thread", result.stdout)
        self.assertIn(
            "No thread with name \"nonexistent_thread\" found in the AOS configuration",
            result.stdout)

    def test_double_ignore_thread(self):
        """Test that ignoring the same thread twice fails appropriately."""
        result = run_test_binary(
            test_mode="double_ignore",
            thread_name="test_thread",
            expect_error=True,
        )

        self.assertIn("First ignore of thread: test_thread", result.stdout)
        self.assertIn("Second ignore of same thread: test_thread",
                      result.stdout)
        self.assertIn(
            "Ignoring the same thread (test_thread) twice. Likely a mistake.",
            result.stdout)

    def test_configure_ignored_thread(self):
        """Test behavior when configuring a thread that was ignored."""
        result = run_test_binary(
            test_mode="configure_ignored",
            thread_name="test_thread",
            expect_error=True,
            # Make sure the main thread doesn't shut down before the RT
            # thread has a chance to crash.
            delay_before_shutdown_ms=100000,
        )

        self.assertIn("Ignoring thread: test_thread", result.stdout)
        self.assertIn("Attempting to configure ignored thread: test_thread",
                      result.stdout)
        self.assertIn(
            "Cannot configure thread test_thread that was already ignored.",
            result.stdout)

    def test_ignore_configured_thread(self):
        """Test behavior when ignoring a thread that was configured."""
        result = run_test_binary(
            test_mode="ignore_configured",
            thread_name="test_thread",
            expect_error=True,
            # Make sure the main thread doesn't shut down before the RT
            # thread has a chance to crash.
            delay_before_shutdown_ms=100000,
        )

        self.assertIn("Configuring thread: test_thread", result.stdout)
        self.assertIn("Ignoring configured thread: test_thread", result.stdout)
        self.assertIn(
            "Cannot ignore thread test_thread that was already configured.",
            result.stdout)

    def test_ignore_thread_from_non_main_thread(self):
        """Test that ignoring a thread from a non-main thread causes a crash."""
        result = run_test_binary(
            test_mode="ignore_thread_from_non_main_thread",
            thread_name="test_thread",
            expect_error=True,
            lock_to_main_thread=True,
        )

        self.assertIn("Configuring thread: test_thread", result.stdout)
        self.assertIn("Call from the main thread instead", result.stdout)

    @unittest.skipIf(EVENT_LOOP_TYPE == "shm",
                     "This test is only supported in simulation.")
    def test_multiple_runs_works_in_simulation(self):
        """Test that we can run the event loop multiple times in simulation."""
        result = run_test_binary(
            test_mode="multiple_runs_works_in_simulation",
            thread_name="test_thread",
            expect_error=False,
        )

        self.assertIn("Configuring thread: test_thread", result.stdout)
        self.assertIn("Thread work complete", result.stdout)
        self.assertEqual(
            1, result.stdout.count("Waiting 20s for 1 thread to start"),
            result.stdout)
        self.assertEqual(3, result.stdout.count("Triggering Run."),
                         result.stdout)

    def test_on_run_unblocks_with_thread(self):
        """Test that OnRun unblocks with thread."""
        for direction in ["acquire", "release"]:
            with self.subTest(direction=direction):
                result = run_test_binary(
                    test_mode="on_run_unblocks_with_thread",
                    thread_name="test_thread",
                    expect_error=False,
                    on_run_direction=direction,
                )

                self.assertIn("Configuring thread: test_thread", result.stdout)
                self.assertIn("Thread work completed", result.stdout)


if __name__ == "__main__":
    unittest.main()
