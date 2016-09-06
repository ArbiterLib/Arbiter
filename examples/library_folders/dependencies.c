#include "dependencies.h"

#include "asprintf.h"
#include "strerror.h"
#include "string_value.h"

#include <arbiter/Requirement.h>
#include <arbiter/Types.h>

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

  char *dependenciesPath;
  if (custom_asprintf(&dependenciesPath, "%s/%s/Dependencies", projectPath, versionPath) < 0) {
    assert(!dependenciesPath);

    *error = strerror_copy("Could not allocate space for path", errno);
    return NULL;
  }

  ArbiterDependencyList *result = create_dependency_list_from_path(dependenciesPath, error);
  free(dependenciesPath);

  return result;
}

ArbiterDependencyList *create_dependency_list_from_path (const char *path, char **error)
{
  assert(error);

  ArbiterDependencyList *result = NULL;
  FILE *dependenciesFd = NULL;

  ArbiterDependency **dependencies = NULL;
  size_t dependenciesCount = 0;

  char buffer[lineReadLength];

  dependenciesFd = fopen(path, "r");
  if (!dependenciesFd) {
    *error = strerror_copy("Could not open Dependencies file", errno);
    goto cleanup;
  }

  while (fgets(buffer, lineReadLength, dependenciesFd)) {
    char *newline = strchr(buffer, '\n');
    if (newline) {
      *newline = '\0';
    }

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

      ArbiterSemanticVersion *semanticVersion = ArbiterCreateSemanticVersionFromString(version);
      if (!semanticVersion) {
        custom_asprintf(error, "Could not parse semantic version: %s", version);
        goto cleanup;
      }

      switch (type) {
        case AtLeast:
          requirement = ArbiterCreateRequirementAtLeast(semanticVersion);
          break;

        case CompatibleWith:
          requirement = ArbiterCreateRequirementCompatibleWith(semanticVersion, ArbiterRequirementStrictnessAllowVersionZeroPatches);
          break;

        case Exactly:
          requirement = ArbiterCreateRequirementExactly(semanticVersion);
          break;

        case Any:
          assert(false);
      }

      ArbiterFree(semanticVersion);
    }

    ArbiterUserValue dependencyValue = string_value_from_string(dependencyPath, strlen(dependencyPath));
    ArbiterProjectIdentifier *dependencyProject = ArbiterCreateProjectIdentifier(dependencyValue);

    dependencies[dependenciesCount - 1] = ArbiterCreateDependency(dependencyProject, requirement);

    ArbiterFree(dependencyProject);
    ArbiterFree(requirement);
  }

  if (ferror(dependenciesFd)) {
    *error = strerror_copy("Error reading Dependencies file", errno);
    goto cleanup;
  }

  result = ArbiterCreateDependencyList((const ArbiterDependency * const *)dependencies, dependenciesCount);

cleanup:
  for (size_t i = 0; i < dependenciesCount; i++) {
    ArbiterFree(dependencies[i]);
  }

  free(dependencies);

  if (dependenciesFd) {
    fclose(dependenciesFd);
  }

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

    ArbiterSemanticVersion *semanticVersion = ArbiterCreateSemanticVersionFromString(name);
    if (!semanticVersion) {
      continue;
    }

    ArbiterUserValue versionValue = string_value_from_string(name, strlen(name));
    ArbiterSelectedVersion *selectedVersion = ArbiterCreateSelectedVersion(semanticVersion, versionValue);

    ArbiterFree(semanticVersion);

    void *newVersions = realloc(versions, sizeof(*versions) * (++versionsCount));
    if (!newVersions) {
      ArbiterFree(selectedVersion);

      *error = strerror_copy("Error reallocating versions list", errno);
      goto cleanup;
    }

    versions = newVersions;
    versions[versionsCount - 1] = selectedVersion;
  }

  result = ArbiterCreateSelectedVersionList((const ArbiterSelectedVersion * const *)versions, versionsCount);

cleanup:
  for (size_t i = 0; i < versionsCount; i++) {
    ArbiterFree(versions[i]);
  }

  free(versions);

  closedir(dir);
  return result;
}
