#include "Version.h"

#include <ostream>
#include <sstream>
#include <iostream>

using namespace Arbiter;

Optional<ArbiterSemanticVersion> ArbiterSemanticVersion::fromString (const std::string &versionString)
{
  unsigned major = 0;
  unsigned minor = 0;
  unsigned patch = 0;
  size_t skip = 0;

  int argsRead = sscanf(versionString.c_str(), "%u.%u.%u%zn", &major, &minor, &patch, &skip);
  if (argsRead < 3) {
    return Optional<ArbiterSemanticVersion>();
  }

  Optional<std::string> prereleaseVersion;
  Optional<std::string> buildMetadata;

  if (skip < versionString.length()) {
    std::istringstream stream(versionString.substr(skip));

    char ch;
    stream >> ch;
    if (stream.good()) {
      if (ch == '-') {
        std::string prerelease;
        std::getline(stream, prerelease, '+');
        if (stream.fail()) {
          return Optional<ArbiterSemanticVersion>();
        }

        // TODO: Verify format of `prerelease`

        prereleaseVersion = Optional<std::string>(std::move(prerelease));
        if (!stream.eof()) {
          ch = '+';
        }
      }

      if (ch == '+') {
        std::string metadata;
        stream >> metadata;
        if (stream.fail()) {
          return Optional<ArbiterSemanticVersion>();
        }

        // TODO: Verify format of `metadata`

        buildMetadata = Optional<std::string>(std::move(metadata));
      } else if (stream.good()) {
        // Unrecognized part of the string
        return Optional<ArbiterSemanticVersion>();
      }
    }
  }

  return ArbiterSemanticVersion(major, minor, patch, prereleaseVersion, buildMetadata);
}

bool ArbiterSemanticVersion::operator< (const ArbiterSemanticVersion &other) const noexcept
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

std::ostream &operator<< (std::ostream &os, const ArbiterSemanticVersion &version)
{
  os << version._major << '.' << version._minor << '.' << version._patch;

  if (version._prereleaseVersion) {
    os << '-' << version._prereleaseVersion.value();
  }

  if (version._buildMetadata) {
    os << '+' << version._buildMetadata.value();
  }

  return os;
}

std::ostream &operator<< (std::ostream &os, const ArbiterSelectedVersion &version)
{
  return os << version._semanticVersion << " (" << version._metadata << ")";
}

ArbiterSemanticVersion *ArbiterCreateSemanticVersion (unsigned major, unsigned minor, unsigned patch, const char *prereleaseVersion, const char *buildMetadata)
{
  return new ArbiterSemanticVersion(
    major,
    minor,
    patch,
    (prereleaseVersion ? Optional<std::string>(prereleaseVersion) : Optional<std::string>()),
    (buildMetadata ? Optional<std::string>(buildMetadata) : Optional<std::string>())
  );
}

ArbiterSemanticVersion *ArbiterCreateSemanticVersionFromString (const char *string)
{
  auto version = ArbiterSemanticVersion::fromString(string);
  if (version) {
    return new ArbiterSemanticVersion(std::move(version.value()));
  } else {
    return nullptr;
  }
}

void ArbiterFreeSemanticVersion (ArbiterSemanticVersion *version)
{
  delete version;
}

unsigned ArbiterGetMajorVersion (const ArbiterSemanticVersion *version)
{
  return version->_major;
}

unsigned ArbiterGetMinorVersion (const ArbiterSemanticVersion *version)
{
  return version->_minor;
}

unsigned ArbiterGetPatchVersion (const ArbiterSemanticVersion *version)
{
  return version->_patch;
}

const char *ArbiterGetPrereleaseVersion (const ArbiterSemanticVersion *version)
{
  if (version->_prereleaseVersion) {
    return version->_prereleaseVersion->c_str();
  } else {
    return nullptr;
  }
}

const char *ArbiterGetBuildMetadata (const ArbiterSemanticVersion *version)
{
  if (version->_buildMetadata) {
    return version->_buildMetadata->c_str();
  } else {
    return nullptr;
  }
}

bool ArbiterEqualVersions (const ArbiterSemanticVersion *lhs, const ArbiterSemanticVersion *rhs)
{
  return *lhs == *rhs;
}

int ArbiterCompareVersionOrdering (const ArbiterSemanticVersion *lhs, const ArbiterSemanticVersion *rhs)
{
  if (*lhs < *rhs) {
    return -1;
  } else if (*lhs > *rhs) {
    return 1;
  } else {
    return 0;
  }
}

ArbiterSelectedVersion *ArbiterCreateSelectedVersion (const ArbiterSemanticVersion *semanticVersion, ArbiterUserValue metadata)
{
  return new ArbiterSelectedVersion(*semanticVersion, SharedUserValue(metadata));
}

const ArbiterSemanticVersion *ArbiterSelectedVersionSemanticVersion (const ArbiterSelectedVersion *version)
{
  return &version->_semanticVersion;
}

const void *ArbiterSelectedVersionMetadata (const ArbiterSelectedVersion *version)
{
  return version->_metadata.data();
}

bool ArbiterEqualSelectedVersions (const ArbiterSelectedVersion *lhs, const ArbiterSelectedVersion *rhs)
{
  return *lhs == *rhs;
}

void ArbiterFreeSelectedVersion (ArbiterSelectedVersion *version)
{
  delete version;
}
