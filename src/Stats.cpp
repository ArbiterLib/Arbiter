#include "Stats.h"

namespace Arbiter {

std::ostream &operator<< (std::ostream &os, const Stats &stats)
{
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(*stats._endTime - *stats._startTime);

  return os
    << "Duration: " << ms.count() << "ms\n"
    << "Available version fetches: " << stats._availableVersionFetches << "\n"
    << "Dependency list fetches: " << stats._dependencyListFetches << "\n"
    << "Cached available versions size: ~" << stats._cachedAvailableVersionsSizeEstimate << " bytes (excl. user data)\n"
    << "Cached dependency lists size: ~" << stats._cachedDependenciesSizeEstimate << " bytes (excl. user data)\n"
    << "Dead ends encountered: " << stats._deadEnds;
}

} // namespace Arbiter
