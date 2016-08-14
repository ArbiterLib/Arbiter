#ifndef ARBITER_REQUIREMENT_H
#define ARBITER_REQUIREMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Version.h"

#include <stdbool.h>

/**
 * How strict to be in matching compatible versions.
 */
enum ArbiterRequirementStrictness
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
};

/**
 * Represents a requirement for a specific version or set of versions.
 */
typedef struct ArbiterRequirement ArbiterRequirement;

ArbiterRequirement *ArbiterCreateRequirementAny (void);
ArbiterRequirement *ArbiterCreateRequirementAtLeast (const ArbiterSemanticVersion *version);
ArbiterRequirement *ArbiterCreateRequirementCompatibleWith (const ArbiterSemanticVersion *version, ArbiterRequirementStrictness strictness);
ArbiterRequirement *ArbiterCreateRequirementExactly (const ArbiterSemanticVersion *version);
void ArbiterFreeRequirement (ArbiterRequirement *requirement);

bool ArbiterEqualRequirements (const ArbiterRequirement *lhs, const ArbiterRequirement *rhs);
bool ArbiterRequirementSatisfiedBy (const ArbiterRequirement *requirement, const ArbiterSemanticVersion *version);

#ifdef __cplusplus
}
#endif

#endif
