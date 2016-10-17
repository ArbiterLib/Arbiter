#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <chrono>
#include <ostream>

#include "Optional.h"

namespace Arbiter {

/**
 * Used for benchmarking various aspects of dependency resolution.
 */
struct Stats final
{
  public:
    using Clock = std::chrono::steady_clock;

    Stats () = default;

    Stats (Clock::time_point startTime)
      : _startTime(std::move(startTime))
    {}

    unsigned _deadEnds{0};
    unsigned _availableVersionFetches{0};
    unsigned _dependencyListFetches{0};
    size_t _cachedDependenciesSizeEstimate{0};
    size_t _cachedAvailableVersionsSizeEstimate{0};
    Optional<Clock::time_point> _startTime;
    Optional<Clock::time_point> _endTime;
};

std::ostream &operator<< (std::ostream &os, const Stats &stats);

} // namespace Arbiter
