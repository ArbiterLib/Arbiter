#include "gtest/gtest.h"

#include "Dependency.h"
#include "Requirement.h"
#include "Resolver.h"

#include "TestValue.h"

#include <algorithm>
#include <fstream>
#include <iostream>

using namespace Arbiter;
using namespace Testing;

namespace {

const std::string baseDirectory = "test/fixtures/carthage-graph/";

Optional<ArbiterSemanticVersion> semanticVersionFromString (std::string version)
{
  if (version.front() == 'v') {
    version.erase(0, 1);
  }

  // problem???
  while (std::count(version.begin(), version.end(), '.') < 2) {
    version += ".0";
  }

  return ArbiterSemanticVersion::fromString(version);
}

ArbiterDependencyList loadDependencyList (const std::string &projectName, const std::string &versionString)
{
  std::ifstream fd(baseDirectory + projectName + "/" + versionString + ".txt");
  std::vector<ArbiterDependency> dependencies;

  while (fd) {
    std::string depName;
    std::string constraint;
    std::string version;

    fd >> depName >> constraint >> version;

    auto semanticVersion = semanticVersionFromString(std::move(version));
    if (!semanticVersion) {
      continue;
    }

    std::unique_ptr<ArbiterRequirement> requirement;
    if (constraint == "==") {
      requirement = std::make_unique<Requirement::Exactly>(std::move(*semanticVersion));
    } else if (constraint == "~>") {
      requirement = std::make_unique<Requirement::CompatibleWith>(std::move(*semanticVersion), ArbiterRequirementStrictnessAllowVersionZeroPatches);
    } else if (constraint == ">=") {
      requirement = std::make_unique<Requirement::AtLeast>(std::move(*semanticVersion));
    } else {
      // TODO: 'Any' constraints
      throw std::runtime_error("Unrecognized constraint: " + constraint);
    }

    ArbiterProjectIdentifier depProject(makeSharedUserValue<ArbiterProjectIdentifier, StringTestValue>(std::move(depName)));
    dependencies.emplace_back(std::move(depProject), *requirement);
  }

  return ArbiterDependencyList(std::move(dependencies));
}

ArbiterDependencyList *createDependencyList (const ArbiterResolver *, const ArbiterProjectIdentifier *project, const ArbiterSelectedVersion *version, char **)
{
  return new ArbiterDependencyList(loadDependencyList(fromUserValue<StringTestValue>(project->_value.data())._str, fromUserValue<StringTestValue>(version->_metadata.data())._str));
}

ArbiterSelectedVersionList *createAvailableVersionsList (const ArbiterResolver *, const ArbiterProjectIdentifier *project, char **error)
{
  std::string projectName = fromUserValue<StringTestValue>(project->_value.data())._str;
  std::ifstream fd(baseDirectory + projectName + ".txt");

  if (!fd) {
    *error = strdup(std::string("No version list found for project: " + projectName).c_str());
    return nullptr;
  }

  std::vector<ArbiterSelectedVersion> versions;

  do {
    std::string versionStr;
    fd >> versionStr;

    auto semanticVersion = semanticVersionFromString(versionStr);
    if (!semanticVersion) {
      continue;
    }

    versions.emplace_back(std::move(semanticVersion), makeSharedUserValue<ArbiterSelectedVersion, StringTestValue>(std::move(versionStr)));
  } while (fd);

  return new ArbiterSelectedVersionList(std::move(versions));
}

} // namespace

TEST(CarthageGraphTest, ResolvesCorrectly) {
  ArbiterResolverBehaviors behaviors{&createDependencyList, &createAvailableVersionsList, nullptr};
  ArbiterResolver resolver(behaviors, ArbiterResolvedDependencyGraph(), loadDependencyList("Carthage", "0.18"), nullptr);

  ArbiterResolvedDependencyGraph resolved = resolver.resolve();
  auto stats = resolver._latestStats;

  std::unordered_map<std::string, ArbiterSemanticVersion> versions;
  for (const auto &pair : resolved.nodes()) {
    ArbiterResolvedDependency dep = ArbiterResolvedDependencyGraph::resolveNode(pair);

    std::string projectName = fromUserValue<StringTestValue>(dep._project._value.data())._str;
    ArbiterSemanticVersion version = *dep._version._semanticVersion;

    versions.emplace(std::make_pair(std::move(projectName), std::move(version)));
  }

  EXPECT_EQ(versions.size(), 6);
  EXPECT_EQ(versions.at("PrettyColors"), ArbiterSemanticVersion(4, 0, 0));
  EXPECT_EQ(versions.at("Result"), ArbiterSemanticVersion(2, 1, 3));
  EXPECT_EQ(versions.at("Commandant"), ArbiterSemanticVersion(0, 10, 1));
  EXPECT_EQ(versions.at("ReactiveCocoa"), ArbiterSemanticVersion(4, 2, 2));
  EXPECT_EQ(versions.at("ReactiveTask"), ArbiterSemanticVersion(0, 10, 3));
  EXPECT_EQ(versions.at("Tentacle"), ArbiterSemanticVersion(0, 4, 1));

  // TODO: Verify install-ordered graph

  std::cout << "*** Statistics ***\n" << stats << std::endl;
}
