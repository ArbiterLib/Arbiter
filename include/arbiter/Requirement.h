#ifndef ARBITER_REQUIREMENT_H
#define ARBITER_REQUIREMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Version.h>

#include <stdbool.h>

/**
 * How strict to be in matching compatible versions.
 */
typedef enum
{
  /**
   * Determine compatibility according to a strict interpretation of SemVer.
   */
  ArbiterRequirementStrictnessStrict,

  /**
   * According to SemVer, technically all 0.y.z releases can break backwards
   * compatibility, meaning that minor and patch versions have to match
   * exactly in order to be "compatible."
   *
   * This looser variant permits newer patch versions, which is probably
   * closer to what the user wants.
   */
  ArbiterRequirementStrictnessAllowVersionZeroPatches,
} ArbiterRequirementStrictness;

/**
 * Represents a requirement for a specific version or set of versions.
 */
typedef struct ArbiterRequirement ArbiterRequirement;

/**
 * Creates a requirement which will match any version.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementAny (void);

/**
 * Creates a requirement which will match versions not less than the specified
 * version.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementAtLeast (const ArbiterSemanticVersion *version);

/**
 * Creates a requirement which will match versions that are "compatible with"
 * the given version, according to Semantic Versioning rules about backwards
 * compatibility.
 *
 * Exceptions to the SemVer rules can be applied by using a value other than
 * `ArbiterRequirementStrictnessStrict` for `strictness`.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementCompatibleWith (const ArbiterSemanticVersion *version, ArbiterRequirementStrictness strictness);

/**
 * Creates a requirement which will only match the specified version, including
 * any prerelease version and build metadata.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementExactly (const ArbiterSemanticVersion *version);

/**
 * Determines whether the given requirement is satisfied by the given version.
 */
bool ArbiterRequirementSatisfiedBy (const ArbiterRequirement *requirement, const ArbiterSemanticVersion *version);

#ifdef __cplusplus
}
#endif

#endif
