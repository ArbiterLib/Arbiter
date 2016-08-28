#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Optional.h"

#include <type_traits>

namespace Arbiter {

/**
 * Looks up a key in a map, returning the corresponding value if it exists.
 */
template<typename Map, typename Key, typename Value = typename Map::mapped_type>
Optional<Value> maybeAt (const Map &map, const Key &key)
{
  auto it = map.find(key);
  if (it == map.end()) {
    return None();
  } else {
    return makeOptional(it->second);
  }
}

} // namespace Arbiter
