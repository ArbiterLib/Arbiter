#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Version.h>

#include "Optional.h"
#include "Types.h"
#include "Value.h"

#include <functional>
#include <string>
#include <vector>

struct ArbiterSemanticVersion final : public Arbiter::Base
{
  public:
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

    /**
     * Attempts to parse a well-formed semantic version from a string.
     */
    // TODO: Add error reporting
    static Arbiter::Optional<ArbiterSemanticVersion> fromString (const std::string &versionString);

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

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

struct ArbiterSelectedVersion final : public Arbiter::Base
{
  public:
    using Metadata = Arbiter::SharedUserValue<ArbiterSelectedVersion>;

    Arbiter::Optional<ArbiterSemanticVersion> _semanticVersion;
    Metadata _metadata;

    ArbiterSelectedVersion (Arbiter::Optional<ArbiterSemanticVersion> semanticVersion, Metadata metadata)
      : _semanticVersion(std::move(semanticVersion))
      , _metadata(std::move(metadata))
    {}

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

    bool operator< (const ArbiterSelectedVersion &other) const;

    bool operator> (const ArbiterSelectedVersion &other) const
    {
      return other < *this;
    }

    bool operator<= (const ArbiterSelectedVersion &other) const
    {
      return !(*this > other);
    }

    bool operator>= (const ArbiterSelectedVersion &other) const
    {
      return !(*this < other);
    }
};

struct ArbiterSelectedVersionList final : public Arbiter::Base
{
  public:
    std::vector<ArbiterSelectedVersion> _versions;

    ArbiterSelectedVersionList () = default;

    explicit ArbiterSelectedVersionList (std::vector<ArbiterSelectedVersion> versions)
      : _versions(std::move(versions))
    {}

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
};

namespace std {

template<>
struct hash<ArbiterSemanticVersion> final
{
  public:
    size_t operator() (const ArbiterSemanticVersion &version) const;
};

template<>
struct hash<ArbiterSelectedVersion> final
{
  public:
    size_t operator() (const ArbiterSelectedVersion &version) const;
};

} // namespace std
