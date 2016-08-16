#ifndef ARBITER_VERSION_H
#define ARBITER_VERSION_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a semantic version, as defined by semver.org.
 */
typedef struct ArbiterSemanticVersion ArbiterSemanticVersion;

/**
 * Creates a semantic version with the given components.
 *
 * The returned version must be freed with ArbiterFreeSemanticVersion().
 */
ArbiterSemanticVersion *ArbiterCreateSemanticVersion (unsigned major, unsigned minor, unsigned patch, const char *prereleaseVersion, const char *buildMetadata);

/**
 * Attempts to parse the given NUL-terminated string into a semantic version,
 * returning NULL if a parse failure occurs.
 *
 * The returned version must be freed with ArbiterFreeSemanticVersion().
 */
ArbiterSemanticVersion *ArbiterCreateSemanticVersionFromString (const char *string);

/**
 * Releases the memory associated with a version object.
 */
void ArbiterFreeSemanticVersion (ArbiterSemanticVersion *version);

/**
 * Returns the major version number (X.y.z) from a semantic version.
 */
unsigned ArbiterGetMajorVersion (const ArbiterSemanticVersion *version);

/**
 * Returns the minor version number (x.Y.z) from a semantic version.
 */
unsigned ArbiterGetMinorVersion (const ArbiterSemanticVersion *version);

/**
 * Returns the patch version number (x.y.Z) from a semantic version.
 */
unsigned ArbiterGetPatchVersion (const ArbiterSemanticVersion *version);

/**
 * Returns the prerelease version string from a semantic version, or NULL if
 * there is not one associated with the version.
 *
 * For example, in the version `1.0.0-alpha.1`, the prerelease version string
 * will be `alpha.1`.
 */
const char *ArbiterGetPrereleaseVersion (const ArbiterSemanticVersion *version);

/**
 * Returns the build metadata string from a semantic version, or NULL if there
 * is not one associated with the version.
 *
 * For example, in the version `1.0.0+20160814`, the build metadata string will
 * be `20160814`.
 */
const char *ArbiterGetBuildMetadata (const ArbiterSemanticVersion *version);

/**
 * Checks whether two versions are equal in every component, including those
 * which may not participate in ordering (e.g., build metadata).
 */
bool ArbiterEqualVersions (const ArbiterSemanticVersion *lhs, const ArbiterSemanticVersion *rhs);

/**
 * Orders two semantic versions relative to each other.
 *
 * Returns -1 if `lhs` is less than `rhs`, 1 if `lhs` is greater than `rhs`, or
 * 0 if the two versions have the same precedence (which may be the case even if
 * their build metadata differs).
 */
int ArbiterCompareVersionOrdering (const ArbiterSemanticVersion *lhs, const ArbiterSemanticVersion *rhs);

typedef struct ArbiterSelectedVersion ArbiterSelectedVersion;

ArbiterSelectedVersion *ArbiterCreateSelectedVersion (const ArbiterSemanticVersion *semanticVersion, ArbiterUserValue metadata);
const ArbiterSemanticVersion *ArbiterSelectedVersionSemanticVersion (const ArbiterSelectedVersion *version);
const void *ArbiterSelectedVersionMetadata (const ArbiterSelectedVersion *version);
bool ArbiterEqualSelectedVersions (const ArbiterSelectedVersion *lhs, const ArbiterSelectedVersion *rhs);
void ArbiterFreeSelectedVersion (ArbiterSelectedVersion *version);

#ifdef __cplusplus
}
#endif

#endif
