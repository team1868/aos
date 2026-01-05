#include <sched.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "absl/log/absl_check.h"

#include "aos/realtime.h"

namespace aos {

CpuSet::CpuSet() { CPU_ZERO(&set_); }

void CpuSet::Set(int cpu) { CPU_SET(cpu, &set_); }

void CpuSet::Clear(int cpu) { CPU_CLR(cpu, &set_); }

void CpuSet::Clear() { CPU_ZERO(&set_); }

bool CpuSet::IsSet(int cpu) const { return CPU_ISSET(cpu, &set_); }

bool CpuSet::Empty() const { return CPU_COUNT(&set_) == 0; }

bool CpuSet::operator==(const CpuSet &other) const {
  return CPU_EQUAL(&set_, &other.set_);
}

bool CpuSet::operator!=(const CpuSet &other) const {
  return !CPU_EQUAL(&set_, &other.set_);
}

void SetCurrentThreadAffinity(const CpuSet &cpuset) {
  ABSL_PCHECK(sched_setaffinity(0, sizeof(cpu_set_t), cpuset.native_handle()) ==
              0)
      << cpuset;
}

CpuSet GetCurrentThreadAffinity() {
  CpuSet result;
  ABSL_PCHECK(sched_getaffinity(0, sizeof(cpu_set_t), result.native_handle()) ==
              0);
  return result;
}

}  // namespace aos
