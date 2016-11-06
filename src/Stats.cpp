#include "Stats.h"

namespace Arbiter {

std::ostream &operator<< (std::ostream &os, const Stats &stats)
{
  std::chrono::milliseconds ms;
  if (stats._startTime && stats._endTime) {
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(*stats._endTime - *stats._startTime);
  } else {
    ms = std::chrono::milliseconds(0);
  }

  return os
    << "Duration: " << ms.count() << "ms\n"
    << "Available version fetches: " << stats._availableVersionFetches << "\n"
    << "Dependency list fetches: " << stats._dependencyListFetches << "\n"
    << "Cached available versions size: ~" << stats._cachedAvailableVersionsSizeEstimate << " bytes (excl. user data)\n"
    << "Cached dependency lists size: ~" << stats._cachedDependenciesSizeEstimate << " bytes (excl. user data)\n"
    << "Dead ends encountered: " << stats._deadEnds;
}

} // namespace Arbiter
