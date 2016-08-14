#ifndef ARBITER_VERSION_INL_H
#define ARBITER_VERSION_INL_H

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Version.h"
#include "internal/Optional.h"

#include <string>

struct ArbiterSemanticVersion
{
  unsigned _major;
  unsigned _minor;
  unsigned _patch;

  Arbiter::Optional<std::string> _prereleaseVersion;
  Arbiter::Optional<std::string> _buildMetadata;

  ArbiterSemanticVersion (unsigned major, unsigned minor, unsigned patch, Arbiter::Optional<std::string> prereleaseVersion = Arbiter::Optional<std::string>(), Arbiter::Optional<std::string> buildMetadata = Arbiter::Optional<std::string>())
    : _major(major)
    , _minor(minor)
    , _patch(patch)
    , _prereleaseVersion(prereleaseVersion)
    , _buildMetadata(buildMetadata)
  {}

  // TODO: Add error reporting
  static Arbiter::Optional<ArbiterSemanticVersion> fromString (const std::string &versionString);

  bool operator== (const ArbiterSemanticVersion &other) const noexcept
  {
    return _major == other._major && _minor == other._minor && _patch == other._patch && _prereleaseVersion == other._prereleaseVersion && _buildMetadata == other._buildMetadata;
  }

  bool operator< (const ArbiterSemanticVersion &other) const noexcept;
};

std::ostream &operator<< (std::ostream &os, const ArbiterSemanticVersion &version);

#endif
