#ifndef ARBITER_VERSION_INL_H
#define ARBITER_VERSION_INL_H

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Value-inl.h"
#include "Version.h"
#include "internal/Hash.h"
#include "internal/Optional.h"

#include <functional>
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

  bool operator!= (const ArbiterSemanticVersion &other) const noexcept
  {
    return !(*this == other);
  }

  bool operator< (const ArbiterSemanticVersion &other) const noexcept;

  bool operator> (const ArbiterSemanticVersion &other) const noexcept
  {
    return other < *this;
  }

  bool operator>= (const ArbiterSemanticVersion &other) const noexcept
  {
    return !(*this < other);
  }

  bool operator<= (const ArbiterSemanticVersion &other) const noexcept
  {
    return other >= *this;
  }
};

std::ostream &operator<< (std::ostream &os, const ArbiterSemanticVersion &version);

struct ArbiterSelectedVersion
{
  public:
    ArbiterSemanticVersion _semanticVersion;
    Arbiter::SharedUserValue _metadata;

    ArbiterSelectedVersion (ArbiterSemanticVersion semanticVersion, Arbiter::SharedUserValue metadata)
      : _semanticVersion(std::move(semanticVersion))
      , _metadata(std::move(metadata))
    {}

    bool operator== (const ArbiterSelectedVersion &other) const
    {
      return _semanticVersion == other._semanticVersion && _metadata == other._metadata;
    }
};

std::ostream &operator<< (std::ostream &os, const ArbiterSelectedVersion &version);

namespace std {

template<>
struct hash<ArbiterSemanticVersion>
{
  public:
    size_t operator() (const ArbiterSemanticVersion &version) const
    {
      return Arbiter::hashOf(version._major)
        ^ Arbiter::hashOf(version._minor)
        ^ Arbiter::hashOf(version._patch)
        ^ Arbiter::hashOf(version._prereleaseVersion)
        ^ Arbiter::hashOf(version._buildMetadata);
    }
};

template<>
struct hash<ArbiterSelectedVersion>
{
  public:
    size_t operator() (const ArbiterSelectedVersion &version) const
    {
      return Arbiter::hashOf(version._semanticVersion);
    }
};

}

#endif
