#ifndef ARBITER_VERSION_H
#define ARBITER_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ArbiterSemanticVersion ArbiterSemanticVersion;

unsigned ArbiterGetMajorVersion (const ArbiterSemanticVersion *version);
unsigned ArbiterGetMinorVersion (const ArbiterSemanticVersion *version);
unsigned ArbiterGetPatchVersion (const ArbiterSemanticVersion *version);
const char *ArbiterGetPrereleaseVersion (const ArbiterSemanticVersion *version);
const char *ArbiterGetBuildMetadata (const ArbiterSemanticVersion *version);

#ifdef __cplusplus
}
#endif

#endif
