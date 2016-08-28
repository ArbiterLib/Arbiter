#include "Dependency.h"
#include "Requirement.h"
#include "Resolver.h"
#include "ToString.h"
#include "Value.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <stdexcept>

using namespace Arbiter;
using namespace Resolver;

namespace {

/**
 * A C++ object capable of being an ArbiterUserValue.
 */
class TestValue
{
  public:
    virtual ~TestValue () = default;

    virtual bool operator== (const TestValue &) const = 0;
    virtual bool operator< (const TestValue &) const = 0;
    virtual std::ostream &describe (std::ostream &os) const = 0;

    static ArbiterUserValue convertToUserValue (std::unique_ptr<TestValue> testValue)
    {
      ArbiterUserValue userValue;
      userValue.data = testValue.release();
      userValue.equalTo = &equalTo;
      userValue.lessThan = &lessThan;
      userValue.destructor = &destructor;
      userValue.createDescription = &createDescription;
      return userValue;
    }

  private:
    static bool equalTo (const void *first, const void *second)
    {
      return *static_cast<const TestValue *>(first) == *static_cast<const TestValue *>(second);
    }

    static bool lessThan (const void *first, const void *second)
    {
      return *static_cast<const TestValue *>(first) < *static_cast<const TestValue *>(second);
    }

    static void destructor (void *data)
    {
      delete static_cast<TestValue *>(data);
    }

    static char *createDescription (const void *data)
    {
      return copyCString(toString(*static_cast<const TestValue *>(data))).release();
    }
};

std::ostream &operator<< (std::ostream &os, const TestValue &value)
{
  return value.describe(os);
}

class EmptyTestValue final : public TestValue
{
  public:
    EmptyTestValue () = default;

    bool operator== (const TestValue &other) const override
    {
      return dynamic_cast<const EmptyTestValue *>(&other);
    }

    bool operator< (const TestValue &other) const override
    {
      return !(*this == other);
    }

    std::ostream &describe (std::ostream &os) const override
    {
      return os << "EmptyTestValue";
    }
};

struct StringTestValue final : public TestValue
{
  public:
    explicit StringTestValue (std::string str)
      : _str(std::move(str))
    {}

    bool operator== (const TestValue &other) const override
    {
      if (auto ptr = dynamic_cast<const StringTestValue *>(&other)) {
        return _str == ptr->_str;
      } else {
        return true;
      }
    }

    bool operator< (const TestValue &other) const override
    {
      if (auto ptr = dynamic_cast<const StringTestValue *>(&other)) {
        return _str < ptr->_str;
      } else {
        return true;
      }
    }

    std::ostream &describe (std::ostream &os) const override
    {
      return os << _str;
    }

  private:
    std::string _str;
};

template<typename Owner, typename Value, typename... Args>
SharedUserValue<Owner> makeSharedUserValue (Args &&...args)
{
  return SharedUserValue<Owner>(TestValue::convertToUserValue(std::make_unique<Value>(std::forward<Args>(args)...)));
}

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

const ArbiterResolvedDependency &findResolved (const ArbiterResolvedDependencyList &list, const std::string &name)
{
  ArbiterProjectIdentifier identifier = makeProjectIdentifier(name);

  auto it = std::find_if(list._dependencies.begin(), list._dependencies.end(), [&identifier](const ArbiterResolvedDependency &dependency) {
    return dependency._project == identifier;
  });

  if (it == list._dependencies.end()) {
    throw std::out_of_range("Dependency " + name + " not found in resolved list");
  }

  return *it;
}

} // namespace

TEST(ResolverTest, ResolvesEmptyDependencies) {
  ArbiterResolverBehaviors behaviors;
  behaviors.createDependencyList = &createEmptyDependencyList;
  behaviors.createAvailableVersionsList = &createEmptyAvailableVersionsList;

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(), makeSharedUserValue<ArbiterResolver, EmptyTestValue>());

  ArbiterResolvedDependencyList resolved = resolver.resolve();
  EXPECT_TRUE(resolved._dependencies.empty());
}

TEST(ResolverTest, ResolvesOneDependency) {
  ArbiterResolverBehaviors behaviors;
  behaviors.createDependencyList = &createEmptyDependencyList;
  behaviors.createAvailableVersionsList = &createMajorVersionsList;

  std::vector<ArbiterDependency> dependencies;
  dependencies.emplace_back(emptyProjectIdentifier(), Requirement::AtLeast(ArbiterSemanticVersion(2, 0, 0)));

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(std::move(dependencies)), makeSharedUserValue<ArbiterResolver, EmptyTestValue>());

  ArbiterResolvedDependencyList resolved = resolver.resolve();
  ASSERT_EQ(resolved._dependencies.size(), 1);
  EXPECT_EQ(resolved._dependencies[0]._project, emptyProjectIdentifier());
  EXPECT_EQ(resolved._dependencies[0]._version._semanticVersion, ArbiterSemanticVersion(3, 0, 0));
}

TEST(ResolverTest, ResolvesMultipleDependencies)
{
  ArbiterResolverBehaviors behaviors;
  behaviors.createDependencyList = &createEmptyDependencyList;
  behaviors.createAvailableVersionsList = &createMajorVersionsList;

  std::vector<ArbiterDependency> dependencies;
  dependencies.emplace_back(makeProjectIdentifier("B"), Requirement::CompatibleWith(ArbiterSemanticVersion(2, 0, 0), ArbiterRequirementStrictnessStrict));
  dependencies.emplace_back(makeProjectIdentifier("A"), Requirement::AtLeast(ArbiterSemanticVersion(2, 0, 1)));
  dependencies.emplace_back(makeProjectIdentifier("C"), Requirement::Exactly(ArbiterSemanticVersion(1, 0, 0)));

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(std::move(dependencies)), makeSharedUserValue<ArbiterResolver, EmptyTestValue>());

  ArbiterResolvedDependencyList resolved = resolver.resolve();
  ASSERT_EQ(resolved._dependencies.size(), 3);
  EXPECT_EQ(findResolved(resolved, "A")._version._semanticVersion, ArbiterSemanticVersion(3, 0, 0));
  EXPECT_EQ(findResolved(resolved, "B")._version._semanticVersion, ArbiterSemanticVersion(2, 0, 0));
  EXPECT_EQ(findResolved(resolved, "C")._version._semanticVersion, ArbiterSemanticVersion(1, 0, 0));
}

TEST(ResolverTest, ResolvesTransitiveDependencies)
{
  ArbiterResolverBehaviors behaviors;
  behaviors.createDependencyList = &createTransitiveDependencyList;
  behaviors.createAvailableVersionsList = &createVariedVersionsList;

  std::vector<ArbiterDependency> dependencies;
  dependencies.emplace_back(makeProjectIdentifier("ancestor"), Requirement::Exactly(ArbiterSemanticVersion(1, 0, 1, makeOptional("alpha"))));
  dependencies.emplace_back(makeProjectIdentifier("parent"), Requirement::CompatibleWith(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessStrict));

  ArbiterResolver resolver(behaviors, ArbiterDependencyList(std::move(dependencies)), makeSharedUserValue<ArbiterResolver, EmptyTestValue>());

  ArbiterResolvedDependencyList resolved = resolver.resolve();
  ASSERT_EQ(resolved._dependencies.size(), 6);
  EXPECT_EQ(findResolved(resolved, "ancestor")._version._semanticVersion, ArbiterSemanticVersion(1, 0, 1, makeOptional("alpha")));
  EXPECT_EQ(findResolved(resolved, "middle")._version._semanticVersion, ArbiterSemanticVersion(1, 3, 0));
  EXPECT_EQ(findResolved(resolved, "parent")._version._semanticVersion, ArbiterSemanticVersion(1, 3, 0));
  EXPECT_EQ(findResolved(resolved, "leaf")._version._semanticVersion, ArbiterSemanticVersion(0, 2, 3));
  EXPECT_EQ(findResolved(resolved, "leaf_majors_only")._version._semanticVersion, ArbiterSemanticVersion(2, 0, 0));
  EXPECT_EQ(findResolved(resolved, "leaf_dailybuild")._version._semanticVersion, ArbiterSemanticVersion(2, 1, 0, None(), makeOptional("dailybuild")));
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
