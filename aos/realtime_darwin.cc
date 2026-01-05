#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <pthread.h>
#include <unistd.h>

#include "absl/log/absl_check.h"

#include "aos/realtime.h"

namespace aos {

CpuSet::CpuSet() {}

void CpuSet::Set(int cpu) {
  if (cpu >= 0 && cpu < static_cast<int>(set_.size())) {
    set_.set(cpu);
  }
}

void CpuSet::Clear(int cpu) {
  if (cpu >= 0 && cpu < static_cast<int>(set_.size())) {
    set_.reset(cpu);
  }
}

void CpuSet::Clear() { set_.reset(); }

bool CpuSet::IsSet(int cpu) const {
  if (cpu >= 0 && cpu < static_cast<int>(set_.size())) {
    return set_.test(cpu);
  }
  return false;
}

bool CpuSet::Empty() const { return set_.none(); }

bool CpuSet::operator==(const CpuSet &other) const {
  return set_ == other.set_;
}

bool CpuSet::operator!=(const CpuSet &other) const {
  return set_ != other.set_;
}

namespace {
// Same problem, affinity is tracked differently in OSX, fake it here too.
pthread_key_t kCpuSetKey;
pthread_once_t kCpuSetKeyOnce = PTHREAD_ONCE_INIT;

void FreeCpuSet(void *p) { delete static_cast<CpuSet *>(p); }

void CreateCpuSetKey() {
  ABSL_PCHECK(pthread_key_create(&kCpuSetKey, FreeCpuSet) == 0);
}
}  // namespace

void SetCurrentThreadAffinity(const CpuSet &cpuset) {
  pthread_once(&kCpuSetKeyOnce, CreateCpuSetKey);
  CpuSet *current_affinity =
      static_cast<CpuSet *>(pthread_getspecific(kCpuSetKey));
  if (current_affinity == nullptr) {
    current_affinity = new CpuSet();
    ABSL_PCHECK(pthread_setspecific(kCpuSetKey, current_affinity) == 0);
  }
  *current_affinity = cpuset;

  if (cpuset.Empty() || cpuset == DefaultAffinity()) {
    thread_affinity_policy_data_t policy = {THREAD_AFFINITY_TAG_NULL};
    ABSL_CHECK_EQ(
        thread_policy_set(pthread_mach_thread_np(pthread_self()),
                          THREAD_AFFINITY_POLICY, (thread_policy_t)&policy,
                          THREAD_AFFINITY_POLICY_COUNT),
        KERN_SUCCESS);
  } else {
    integer_t tag = 0;
    // We want to map the cpuset to an affinity tag.  The kernel doesn't give us
    // enough control to do this perfectly, but we can do a decent job by
    // hashing the cpuset.
    for (size_t i = 0; i < CpuSet::kSize; ++i) {
      if (cpuset.IsSet(i)) {
        tag = (tag << 1) ^ i;
      }
    }

    thread_affinity_policy_data_t policy = {tag};

    ABSL_CHECK_EQ(
        thread_policy_set(pthread_mach_thread_np(pthread_self()),
                          THREAD_AFFINITY_POLICY, (thread_policy_t)&policy,
                          THREAD_AFFINITY_POLICY_COUNT),
        KERN_SUCCESS);
  }
}

CpuSet GetCurrentThreadAffinity() {
  CpuSet result;
  // There is no way to query the affinity back from the kernel, so just return
  // what we were last set to.
  pthread_once(&kCpuSetKeyOnce, CreateCpuSetKey);
  CpuSet *current_affinity =
      static_cast<CpuSet *>(pthread_getspecific(kCpuSetKey));
  if (current_affinity == nullptr) {
    result = DefaultAffinity();
  } else {
    result = *current_affinity;
  }
  return result;
}

}  // namespace aos
