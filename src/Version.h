#ifndef ARBITER_VERSION_H
#define ARBITER_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ArbiterSemanticVersion ArbiterSemanticVersion;

ArbiterSemanticVersion *ArbiterCreateSemanticVersion (unsigned major, unsigned minor, unsigned patch, const char *prereleaseVersion, const char *buildMetadata);
ArbiterSemanticVersion *ArbiterCreateSemanticVersionFromString (const char *string);
void ArbiterFreeSemanticVersion (ArbiterSemanticVersion *version);
unsigned ArbiterGetMajorVersion (const ArbiterSemanticVersion *version);
unsigned ArbiterGetMinorVersion (const ArbiterSemanticVersion *version);
unsigned ArbiterGetPatchVersion (const ArbiterSemanticVersion *version);
const char *ArbiterGetPrereleaseVersion (const ArbiterSemanticVersion *version);
const char *ArbiterGetBuildMetadata (const ArbiterSemanticVersion *version);
int ArbiterCompareVersions (const ArbiterSemanticVersion *lhs, const ArbiterSemanticVersion *rhs);

#ifdef __cplusplus
}
#endif

#endif
