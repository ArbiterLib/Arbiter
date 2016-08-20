#ifndef ARBITER_VERSION_H
#define ARBITER_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Value.h>

#include <stdbool.h>

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
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const char *ArbiterGetPrereleaseVersion (const ArbiterSemanticVersion *version);

/**
 * Returns the build metadata string from a semantic version, or NULL if there
 * is not one associated with the version.
 *
 * For example, in the version `1.0.0+20160814`, the build metadata string will
 * be `20160814`.
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
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

/**
 * Represents a "selected" version, which is a concrete choice of a real project
 * version.
 */
typedef struct ArbiterSelectedVersion ArbiterSelectedVersion;

/**
 * Creates a selected version which corresponds to the given semantic version.
 *
 * The returned version must be freed with ArbiterFreeSelectedVersion().
 */
ArbiterSelectedVersion *ArbiterCreateSelectedVersion (const ArbiterSemanticVersion *semanticVersion, ArbiterUserValue metadata);

/**
 * Returns the semantic version which corresponds to the given selected version.
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const ArbiterSemanticVersion *ArbiterSelectedVersionSemanticVersion (const ArbiterSelectedVersion *version);

/**
 * Returns any metadata pointer which was provided to
 * ArbiterCreateSelectedVersion().
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const void *ArbiterSelectedVersionMetadata (const ArbiterSelectedVersion *version);

/**
 * Returns whether the two selected versions are equivalent.
 */
bool ArbiterEqualSelectedVersions (const ArbiterSelectedVersion *lhs, const ArbiterSelectedVersion *rhs);

/**
 * Releases the memory associated with a selected version object.
 */
void ArbiterFreeSelectedVersion (ArbiterSelectedVersion *version);

#ifdef __cplusplus
}
#endif

#endif
