#include "Dependency.h"
#include "Hash.h"
#include "Requirement.h"
#include "Resolver.h"
#include "ToString.h"
#include "Value.h"

#include "TestValue.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <stdexcept>

using namespace Arbiter;
using namespace Testing;

namespace {

ArbiterDependencyList *createEmptyDependencyList (const ArbiterResolver *, const ArbiterProjectIdentifier *, const ArbiterSelectedVersion *, char **)
{
  return new ArbiterDependencyList();
}

ArbiterSelectedVersionList *createEmptyAvailableVersionsList (const ArbiterResolver *, const ArbiterProjectIdentifier *, char **)
{
  return new ArbiterSelectedVersionList();
}

ArbiterSelectedVersionList *createMajorVersionsList (const ArbiterResolver *, const ArbiterProjectIdentifier *, char **)
{
  std::vector<ArbiterSelectedVersion> versions;
  
  versions.emplace_back(ArbiterSemanticVersion(2, 0, 0), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());
  versions.emplace_back(ArbiterSemanticVersion(3, 0, 0), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());
  versions.emplace_back(ArbiterSemanticVersion(1, 0, 0), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());

  return new ArbiterSelectedVersionList(std::move(versions));
}

ArbiterProjectIdentifier emptyProjectIdentifier ()
{
  return ArbiterProjectIdentifier(makeSharedUserValue<ArbiterProjectIdentifier, EmptyTestValue>());
}

ArbiterProjectIdentifier makeProjectIdentifier (std::string name)
{
  return ArbiterProjectIdentifier(makeSharedUserValue<ArbiterProjectIdentifier, StringTestValue>(std::move(name)));
}

ArbiterSelectedVersionList *createVariedVersionsList (const ArbiterResolver *resolver, const ArbiterProjectIdentifier *project, char **error)
{
  if (*project == makeProjectIdentifier("leaf_majors_only")) {
    return createMajorVersionsList(resolver, project, error);
  }

  std::vector<ArbiterSelectedVersion> versions;
  
  versions.emplace_back(ArbiterSemanticVersion(0, 2, 3), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());
  versions.emplace_back(ArbiterSemanticVersion(1, 0, 1, makeOptional("alpha")), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());
  versions.emplace_back(ArbiterSemanticVersion(1, 0, 1), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());
  versions.emplace_back(ArbiterSemanticVersion(1, 3, 0), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());
  versions.emplace_back(ArbiterSemanticVersion(2, 1, 0, None(), makeOptional("dailybuild")), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>());

  return new ArbiterSelectedVersionList(std::move(versions));
}

ArbiterDependencyList *createTransitiveDependencyList (const ArbiterResolver *, const ArbiterProjectIdentifier *project, const ArbiterSelectedVersion *, char **)
{
  std::vector<ArbiterDependency> dependencies;

  if (*project == makeProjectIdentifier("ancestor")) {
    dependencies.emplace_back(makeProjectIdentifier("middle"), Requirement::CompatibleWith(ArbiterSemanticVersion(1, 0, 1), ArbiterRequirementStrictnessStrict));
    dependencies.emplace_back(makeProjectIdentifier("leaf_majors_only"), Requirement::AtLeast(ArbiterSemanticVersion(1, 0, 0)));
    dependencies.emplace_back(makeProjectIdentifier("leaf_dailybuild"), Requirement::AtLeast(ArbiterSemanticVersion(2, 0, 0)));
  } else if (*project == makeProjectIdentifier("middle")) {
    dependencies.emplace_back(makeProjectIdentifier("leaf_majors_only"), Requirement::Exactly(ArbiterSemanticVersion(2, 0, 0)));
    dependencies.emplace_back(makeProjectIdentifier("leaf"), Requirement::CompatibleWith(ArbiterSemanticVersion(0, 2, 0), ArbiterRequirementStrictnessAllowVersionZeroPatches));
  } else if (*project == makeProjectIdentifier("parent")) {
    dependencies.emplace_back(makeProjectIdentifier("leaf"), Requirement::Exactly(ArbiterSemanticVersion(0, 2, 3)));
    dependencies.emplace_back(makeProjectIdentifier("leaf_dailybuild"), Requirement::CompatibleWith(ArbiterSemanticVersion(2, 1, 0), ArbiterRequirementStrictnessStrict));
  }

  return new ArbiterDependencyList(std::move(dependencies));
}

const ArbiterResolvedDependency &findResolved (const ArbiterResolvedDependencyGraph &graph, size_t depthIndex, const std::string &name)
{
  ArbiterProjectIdentifier identifier = makeProjectIdentifier(name);

  const auto &depth = graph._depths.at(depthIndex);
  auto it = std::find_if(depth.begin(), depth.end(), [&identifier](const ArbiterResolvedDependency &dependency) {
    return dependency._project == identifier;
  });

  if (it == depth.end()) {
    throw std::out_of_range("Dependency " + name + " not found in resolved graph");
  }

  return *it;
}

} // namespace

TEST(ResolverTest, ResolvesEmptyDependencies) {
  ArbiterResolverBehaviors behaviors{&createEmptyDependencyList, &createEmptyAvailableVersionsList, nullptr};

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(), nullptr);

  ArbiterResolvedDependencyGraph resolved = resolver.resolve();
  EXPECT_TRUE(resolved._depths.empty());
  EXPECT_EQ(resolved.depth(), 0);
  EXPECT_EQ(resolved.count(), 0);
}

TEST(ResolverTest, ResolvesOneDependency) {
  ArbiterResolverBehaviors behaviors{&createEmptyDependencyList, &createMajorVersionsList, nullptr};

  std::vector<ArbiterDependency> dependencies;
  dependencies.emplace_back(emptyProjectIdentifier(), Requirement::AtLeast(ArbiterSemanticVersion(2, 0, 0)));

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(std::move(dependencies)), nullptr);

  ArbiterResolvedDependencyGraph resolved = resolver.resolve();
  ASSERT_EQ(resolved.depth(), 1);
  EXPECT_EQ(resolved.count(), 1);
  EXPECT_EQ(resolved._depths.front().begin()->_project, emptyProjectIdentifier());
  EXPECT_EQ(resolved._depths.front().begin()->_version._semanticVersion, makeOptional(ArbiterSemanticVersion(3, 0, 0)));
}

TEST(ResolverTest, ResolvesMultipleDependencies)
{
  ArbiterResolverBehaviors behaviors{&createEmptyDependencyList, &createMajorVersionsList, nullptr};

  std::vector<ArbiterDependency> dependencies;
  dependencies.emplace_back(makeProjectIdentifier("B"), Requirement::CompatibleWith(ArbiterSemanticVersion(2, 0, 0), ArbiterRequirementStrictnessStrict));
  dependencies.emplace_back(makeProjectIdentifier("A"), Requirement::AtLeast(ArbiterSemanticVersion(2, 0, 1)));
  dependencies.emplace_back(makeProjectIdentifier("C"), Requirement::Exactly(ArbiterSemanticVersion(1, 0, 0)));

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(std::move(dependencies)), nullptr);

  ArbiterResolvedDependencyGraph resolved = resolver.resolve();
  ASSERT_EQ(resolved.depth(), 1);
  EXPECT_EQ(resolved.count(), 3);
  EXPECT_EQ(findResolved(resolved, 0, "A")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(3, 0, 0)));
  EXPECT_EQ(findResolved(resolved, 0, "B")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(2, 0, 0)));
  EXPECT_EQ(findResolved(resolved, 0, "C")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(1, 0, 0)));
}

TEST(ResolverTest, ResolvesTransitiveDependencies)
{
  ArbiterResolverBehaviors behaviors{&createTransitiveDependencyList, &createVariedVersionsList, nullptr};

  std::vector<ArbiterDependency> dependencies;
  dependencies.emplace_back(makeProjectIdentifier("ancestor"), Requirement::Exactly(ArbiterSemanticVersion(1, 0, 1, makeOptional("alpha"))));
  dependencies.emplace_back(makeProjectIdentifier("parent"), Requirement::CompatibleWith(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessStrict));

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(std::move(dependencies)), nullptr);

  ArbiterResolvedDependencyGraph resolved = resolver.resolve();
  ASSERT_EQ(resolved.depth(), 3);
  EXPECT_EQ(resolved.count(), 6);
  EXPECT_EQ(findResolved(resolved, 2, "ancestor")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(1, 0, 1, makeOptional("alpha"))));
  EXPECT_EQ(findResolved(resolved, 1, "middle")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(1, 3, 0)));
  EXPECT_EQ(findResolved(resolved, 1, "parent")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(1, 3, 0)));
  EXPECT_EQ(findResolved(resolved, 0, "leaf")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(0, 2, 3)));
  EXPECT_EQ(findResolved(resolved, 0, "leaf_majors_only")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(2, 0, 0)));
  EXPECT_EQ(findResolved(resolved, 0, "leaf_dailybuild")._version._semanticVersion, makeOptional(ArbiterSemanticVersion(2, 1, 0, None(), makeOptional("dailybuild"))));
}

#if 0
TEST(ResolverTest, FailsWhenNoAvailableVersions)
{}

TEST(ResolverTest, FailsWhenNoSatisfyingVersions)
{}

TEST(ResolverTest, FailsWithMutuallyExclusiveRequirements)
{}

TEST(ResolverTest, RethrowsUserDependencyListErrors)
{}

TEST(ResolverTest, RethrowsUserVersionListErrors)
{}
#endif
