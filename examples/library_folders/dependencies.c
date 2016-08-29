#include "dependencies.h"

#include "asprintf.h"
#include "strerror.h"
#include "string_value.h"

#include <arbiter/Requirement.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// not portable
#include <dirent.h>

static const int lineReadLength = 1024;

typedef enum
{
  Any,
  AtLeast,
  CompatibleWith,
  Exactly,
} RequirementType;

ArbiterDependencyList *create_dependency_list (const ArbiterResolver *resolver, const ArbiterProjectIdentifier *project, const ArbiterSelectedVersion *selectedVersion, char **error)
{
  assert(project);
  assert(selectedVersion);
  assert(error);

  const char *projectPath = ArbiterProjectIdentifierValue(project);
  assert(projectPath);

  const char *versionPath = ArbiterSelectedVersionMetadata(selectedVersion);
  assert(versionPath);

  ArbiterDependencyList *result = NULL;
  char *dependenciesPath = NULL;
  FILE *dependenciesFd = NULL;

  ArbiterDependency **dependencies = NULL;
  size_t dependenciesCount = 0;

  char buffer[lineReadLength];

  if (custom_asprintf(&dependenciesPath, "%s/%s/Dependencies", projectPath, versionPath) < 0) {
    assert(!dependenciesPath);

    *error = strerror_copy("Could not allocate space for path", errno);
    goto cleanup;
  }

  dependenciesFd = fopen(dependenciesPath, "r");
  if (!dependenciesFd) {
    *error = strerror_copy("Could not open Dependencies file", errno);
    goto cleanup;
  }

  while (fgets(buffer, lineReadLength, dependenciesFd)) {
    const char *dependencyPath = strtok(buffer, " ");
    if (!dependencyPath) {
      continue;
    }

    void *newDependencies = realloc(dependencies, sizeof(*dependencies) * (++dependenciesCount));
    if (!newDependencies) {
      *error = strerror_copy("Error reallocating dependencies list", errno);
      goto cleanup;
    }

    dependencies = newDependencies;
    dependencies[dependenciesCount - 1] = NULL;

    const char *specifier = strtok(NULL, " ");
    if (!specifier) {
      custom_asprintf(error, "Dependency specified without a requirement: %s", dependencyPath);
      goto cleanup;
    }

    RequirementType type;
    if (strcmp(specifier, "*") == 0) {
      type = Any;
    } else if (strcmp(specifier, ">=") == 0) {
      type = AtLeast;
    } else if (strcmp(specifier, "~>") == 0) {
      type = CompatibleWith;
    } else if (strcmp(specifier, "==") == 0) {
      type = Exactly;
    } else {
      custom_asprintf(error, "Unrecognized requirement specifier: %s", specifier);
      goto cleanup;
    }

    ArbiterRequirement *requirement;

    if (type == Any) {
      requirement = ArbiterCreateRequirementAny();
    } else {
      const char *version = strtok(NULL, " ");
      if (!version) {
        custom_asprintf(error, "Requirement specifier without a version: %s", specifier);
        goto cleanup;
      }

      // TODO: Create appropriate requirement depending on the specifier
      abort();
    }

    ArbiterUserValue dependencyValue = string_value_from_string(dependencyPath, strlen(dependencyPath));
    ArbiterProjectIdentifier *dependencyProject = ArbiterCreateProjectIdentifier(dependencyValue);

    dependencies[dependenciesCount - 1] = ArbiterCreateDependency(dependencyProject, requirement);

    ArbiterFreeProjectIdentifier(dependencyProject);
    ArbiterFreeRequirement(requirement);
  }

  if (ferror(dependenciesFd)) {
    *error = strerror_copy("Error reading Dependencies file", errno);
    goto cleanup;
  }

  result = ArbiterCreateDependencyList((const ArbiterDependency * const *)dependencies, dependenciesCount);

cleanup:
  for (size_t i = 0; i < dependenciesCount; i++) {
    ArbiterFreeDependency(dependencies[i]);
  }

  free(dependencies);

  if (dependenciesFd) {
    fclose(dependenciesFd);
  }

  free(dependenciesPath);
  return result;
}

ArbiterSelectedVersionList *create_available_versions_list (const ArbiterResolver *resolver, const ArbiterProjectIdentifier *project, char **error)
{
  assert(project);
  assert(error);

  const char *projectPath = ArbiterProjectIdentifierValue(project);
  assert(projectPath);

  ArbiterSelectedVersionList *result = NULL;

  ArbiterSelectedVersion **versions = NULL;
  size_t versionsCount = 0;

  DIR *dir = opendir(projectPath);

  struct dirent *dp;
  while ((dp = readdir(dir))) {
    const char *name = dp->d_name;
    size_t len = dp->d_namlen;

    ArbiterSemanticVersion *semanticVersion = ArbiterCreateSemanticVersionFromString(name);
    if (!semanticVersion) {
      continue;
    }

    ArbiterUserValue versionValue = string_value_from_string(name, len);
    ArbiterSelectedVersion *selectedVersion = ArbiterCreateSelectedVersion(semanticVersion, versionValue);

    ArbiterFreeSemanticVersion(semanticVersion);

    void *newVersions = realloc(versions, sizeof(*versions) * (++versionsCount));
    if (!newVersions) {
      ArbiterFreeSelectedVersion(selectedVersion);

      *error = strerror_copy("Error reallocating versions list", errno);
      goto cleanup;
    }

    versions = newVersions;
    versions[versionsCount - 1] = selectedVersion;
  }

  result = ArbiterCreateSelectedVersionList((const ArbiterSelectedVersion * const *)versions, versionsCount);

cleanup:
  for (size_t i = 0; i < versionsCount; i++) {
    ArbiterFreeSelectedVersion(versions[i]);
  }

  free(versions);

  closedir(dir);
  return result;
}
