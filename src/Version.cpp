#include "Version.h"
#include "internal/Optional.h"

#include <ostream>
#include <string>

using namespace Arbiter;

struct ArbiterSemanticVersion
{
  unsigned _major;
  unsigned _minor;
  unsigned _patch;

  Optional<std::string> _prereleaseVersion;
  Optional<std::string> _buildMetadata;

  ArbiterSemanticVersion (unsigned major, unsigned minor, unsigned patch, const std::string *prereleaseVersion = nullptr, const std::string *buildMetadata = nullptr)
    : _major(major)
    , _minor(minor)
    , _patch(patch)
  {
    if (prereleaseVersion) {
      _prereleaseVersion = Optional<std::string>(*prereleaseVersion);
    }

    if (buildMetadata) {
      _buildMetadata = Optional<std::string>(*buildMetadata);
    }
  }

  bool operator== (const ArbiterSemanticVersion &other) const noexcept
  {
    return _major == other._major && _minor == other._minor && _patch == other._patch && _prereleaseVersion == other._prereleaseVersion && _buildMetadata == other._buildMetadata;
  }

  bool operator< (const ArbiterSemanticVersion &other) const noexcept
  {
    if (_major < other._major) {
      return true;
    } else if (_major > other._major) {
      return false;
    }

    if (_minor < other._minor) {
      return true;
    } else if (_minor > other._minor) {
      return false;
    }

    if (_patch < other._patch) {
      return true;
    } else if (_patch > other._patch) {
      return false;
    }

    if (_prereleaseVersion) {
      if (!other._prereleaseVersion) {
        return true;
      }

      // FIXME: This should compare numbers naturally, not lexically
      return _prereleaseVersion.value() < other._prereleaseVersion.value();
    }

    // Build metadata does not participate in precedence.
    return false;
  }
};

std::ostream &operator<< (std::ostream &os, const ArbiterSemanticVersion &version) {
  os << version._major << '.' << version._minor << '.' << version._patch;

  if (version._prereleaseVersion) {
    os << '-' << version._prereleaseVersion.value();
  }

  if (version._buildMetadata) {
    os << '+' << version._buildMetadata.value();
  }

  return os;
}

unsigned ArbiterGetMajorVersion (const ArbiterSemanticVersion *version) {
  return version->_major;
}

unsigned ArbiterGetMinorVersion (const ArbiterSemanticVersion *version) {
  return version->_minor;
}

unsigned ArbiterGetPatchVersion (const ArbiterSemanticVersion *version) {
  return version->_patch;
}

const char *ArbiterGetPrereleaseVersion (const ArbiterSemanticVersion *version) {
  if (version->_prereleaseVersion) {
    return version->_prereleaseVersion->c_str();
  } else {
    return nullptr;
  }
}

const char *ArbiterGetBuildMetadata (const ArbiterSemanticVersion *version) {
  if (version->_buildMetadata) {
    return version->_buildMetadata->c_str();
  } else {
    return nullptr;
  }
}
