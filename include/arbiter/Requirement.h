#ifndef ARBITER_REQUIREMENT_H
#define ARBITER_REQUIREMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Value.h>

#include <stdbool.h>
#include <stddef.h>

// forward declarations
struct ArbiterSemanticVersion;
struct ArbiterSelectedVersion;

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
 * A predicate used to determine whether the given version suitably satisfies
 * the requirement.
 */
typedef bool (*ArbiterRequirementPredicate)(const struct ArbiterSelectedVersion *version, const void *context);

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
ArbiterRequirement *ArbiterCreateRequirementAtLeast (const struct ArbiterSemanticVersion *version);

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
ArbiterRequirement *ArbiterCreateRequirementCompatibleWith (const struct ArbiterSemanticVersion *version, ArbiterRequirementStrictness strictness);

/**
 * Creates a requirement which will only match the specified version, including
 * any prerelease version and build metadata.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementExactly (const struct ArbiterSemanticVersion *version);

/**
 * Creates a requirement which only matches against `ArbiterSelectedVersion`s
 * that have metadata equal to `metadata`.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementUnversioned (ArbiterUserValue metadata);

/**
 * Creates a requirement which will evaluate a custom predicate whenever
 * a specific version is checked against it.
 *
 * The predicate may be invoked many times during dependency resolution, so it
 * should not take a long time to complete.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementCustom (ArbiterRequirementPredicate predicate, ArbiterUserContext context);

/**
 * Creates a compound requirement that evaluates each of a list of requirements.
 * All of the requirements must be satisfied for the compound requirement to be
 * satisfied.
 *
 * The objects in the C array can be safely freed after calling this function.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementCompound (const ArbiterRequirement * const *requirements, size_t count);

/**
 * Creates a requirement with a custom priority, changing how the base
 * requirement intersects with other requirements in the dependency graph.
 *
 * Normally, if two requirements A and B are found for the same project in the
 * graph, they are intersected to create a requirement which satisfies both
 * A and B. If no intersection is possible, dependency resolution fails.
 *
 * Priorities short-circuit this intersection process. If requirement A has
 * a lower _priority index_ (meaning that it is higher priority) than
 * requirement B: requirement A will be used, requirement B will be discarded,
 * and no intersection will be performed.
 *
 * **This can lead to surprising behavior that violates users' expectations**,
 * but is nonetheless occasionally useful. For example, users sometimes want to
 * be able to specify a particular version to use which lies outside of any
 * semantic versioning scheme (e.g., an arbitrary branch or local checkout), in
 * which case it makes sense to disable some semantic version requirements in
 * the dependency graph.
 *
 * _Note:_ prioritized requirements may be used to filter the list of available
 * versions, even if they are lower priority than the default and may get
 * discarded. This means that requirements should avoid rejecting valid versions
 * for the project being considered, or else an unsatisfiable constraints error
 * may result.
 *
 * baseRequirement - A requirement specifying which versions will satisfy the
 *                   new requirement. Must not be NULL.
 * priorityIndex   - A "priority index" for the new requirement. Lower numbers
 *                   indicate higher priority (just like setpriority() on Unix).
 *                   Requirements without an explicit priority set are assumed
 *                   to have priority index 0, meaning negative priorities will
 *                   override the default and positive priorities will _be
 *                   overridden_ by the default.
 *
 * The returned requirement must be freed with ArbiterFree().
 */
ArbiterRequirement *ArbiterCreateRequirementPrioritized (const ArbiterRequirement *baseRequirement, int priorityIndex);

/**
 * Determines whether the given requirement is satisfied by the given version.
 */
bool ArbiterRequirementSatisfiedBy (const ArbiterRequirement *requirement, const struct ArbiterSelectedVersion *version);

/**
 * Returns the priority of the given requirement. See
 * ArbiterCreateRequirementPrioritized() for more information.
 */
int ArbiterRequirementPriority (const ArbiterRequirement *requirement);

#ifdef __cplusplus
}
#endif

#endif
