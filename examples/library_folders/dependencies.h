#pragma once

#include <arbiter/Dependency.h>
#include <arbiter/Resolver.h>
#include <arbiter/Version.h>

ArbiterDependencyList *create_dependency_list (const ArbiterResolver *resolver, const ArbiterProjectIdentifier *project, const ArbiterSelectedVersion *selectedVersion, char **error);

ArbiterDependencyList *create_dependency_list_from_path (const char *path, char **error);

ArbiterSelectedVersionList *create_available_versions_list (const ArbiterResolver *resolver, const ArbiterProjectIdentifier *project, char **error);
