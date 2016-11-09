#include "gtest/gtest.h"

#include "Dependency.h"
#include "Exception.h"
#include "Requirement.h"
#include "Resolver.h"
#include "ToString.h"

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

ArbiterDependencyList *createDependencyList (const ArbiterResolver *, const ArbiterProjectIdentifier *project, const ArbiterSelectedVersion *version, char **error)
{
  try {
    return new ArbiterDependencyList(loadDependencyList(fromUserValue<StringTestValue>(project->_value.data())._str, fromUserValue<StringTestValue>(version->_metadata.data())._str));
  } catch (std::exception &ex) {
    *error = copyCString(ex.what()).release();
    return nullptr;
  }
}

ArbiterSelectedVersionList loadAvailableVersionsList (const std::string &projectName)
{
  std::ifstream fd(baseDirectory + projectName + ".txt");

  if (!fd) {
    throw std::runtime_error("No version list found for project: " + projectName);
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

  return ArbiterSelectedVersionList(std::move(versions));
}

ArbiterSelectedVersionList *createAvailableVersionsList (const ArbiterResolver *, const ArbiterProjectIdentifier *project, char **error)
{
  try {
    return new ArbiterSelectedVersionList(loadAvailableVersionsList(fromUserValue<StringTestValue>(project->_value.data())._str));
  } catch (std::exception &ex) {
    *error = copyCString(ex.what()).release();
    return nullptr;
  }
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

  EXPECT_GE(versions.size(), 6);
  EXPECT_EQ(versions.at("PrettyColors"), ArbiterSemanticVersion(4, 0, 0));
  EXPECT_EQ(versions.at("Result"), ArbiterSemanticVersion(2, 1, 3));
  EXPECT_EQ(versions.at("Commandant"), ArbiterSemanticVersion(0, 10, 1));
  EXPECT_EQ(versions.at("ReactiveCocoa"), ArbiterSemanticVersion(4, 2, 2));
  EXPECT_EQ(versions.at("ReactiveTask"), ArbiterSemanticVersion(0, 10, 3));
  EXPECT_EQ(versions.at("Tentacle"), ArbiterSemanticVersion(0, 4, 1));

  // TODO: Verify install-ordered graph

  std::cout << "*** Statistics ***\n" << stats << std::endl;
}

TEST(CarthageGraphTest, ResolvesAllVersions) {
  ArbiterResolverBehaviors behaviors{&createDependencyList, &createAvailableVersionsList, nullptr};

  std::unordered_set<std::string> brokenVersions = {
    // Versions broken due to revoked dependencies.
    "0.2.2",
    "0.3",
    // Tested above.
    "0.18",
  };

  // TODO: Use a parameterized test instead
  for (const ArbiterSelectedVersion &version : loadAvailableVersionsList("Carthage")._versions) {
    std::string versionString = fromUserValue<StringTestValue>(version._metadata.data())._str;
    if (brokenVersions.find(versionString) != brokenVersions.end()) {
      continue;
    }

    ArbiterResolver resolver(behaviors, ArbiterResolvedDependencyGraph(), loadDependencyList("Carthage", versionString), nullptr);
    try {
      resolver.resolve();
      std::cout << "Carthage " << versionString << ": ✓" << std::endl;
    } catch (const Arbiter::Exception::UserError &ex) {
      std::cout << "Carthage " << versionString << " skipped: " << ex.what() << std::endl;
    } catch (const Arbiter::Exception::Base &ex) {
      EXPECT_TRUE(false) << "Carthage " << versionString << ": " << ex.what();
    }
  }
}
