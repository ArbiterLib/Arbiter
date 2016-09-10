#include "Requirement.h"

#include "TestValue.h"

#include "gtest/gtest.h"

using namespace Arbiter;
using namespace Requirement;
using namespace Testing;

TEST(RequirementTest, AnyRequirement) {
  Any req;
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, Any());
  EXPECT_NE(req, AtLeast(ArbiterSemanticVersion(1, 2, 3)));

  EXPECT_TRUE(req.satisfiedBy(ArbiterSelectedVersion(ArbiterSemanticVersion(0, 0, 0), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>())));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSelectedVersion(ArbiterSemanticVersion(1, 2, 3), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>())));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSelectedVersion(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>())));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSelectedVersion(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")), makeSharedUserValue<ArbiterSelectedVersion, EmptyTestValue>())));
}

TEST(RequirementTest, AtLeastRequirement) {
  AtLeast req(ArbiterSemanticVersion(1, 2, 3));
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, AtLeast(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_NE(req, AtLeast(ArbiterSemanticVersion(1, 2, 4)));
  EXPECT_NE(req, AtLeast(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_NE(req, Any());

  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(2, 3, 4)));
}

TEST(RequirementTest, AtLeastMajorVersionZero) {
  AtLeast req(ArbiterSemanticVersion(0, 0, 1));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 1)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 2)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 1, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
}

TEST(RequirementTest, CompatibleWithRequirement) {
  CompatibleWith req(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessStrict);
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, CompatibleWith(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessStrict));
  EXPECT_NE(req, CompatibleWith(ArbiterSemanticVersion(1, 2, 4), ArbiterRequirementStrictnessStrict));
  EXPECT_NE(req, CompatibleWith(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")), ArbiterRequirementStrictnessStrict));
  EXPECT_NE(req, AtLeast(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_NE(req, Any());

  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 3, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(2, 0, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(2, 3, 4)));
}

TEST(RequirementTest, CompatibleWithMajorVersionZeroStrict) {
  CompatibleWith req(ArbiterSemanticVersion(0, 2, 3), ArbiterRequirementStrictnessStrict);
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3, makeOptional("alpha.1"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4, makeOptional("alpha.1"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 3, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
}

TEST(RequirementTest, CompatibleWithMajorVersionZeroLoose) {
  CompatibleWith req(ArbiterSemanticVersion(0, 2, 3), ArbiterRequirementStrictnessAllowVersionZeroPatches);
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4, makeOptional("alpha.1"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 3, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
}

TEST(RequirementTest, ExactlyRequirement) {
  Exactly req(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")));
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, Exactly(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_NE(req, Exactly(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_NE(req, Exactly(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_NE(req, AtLeast(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_NE(req, Any());

  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(2, 0, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.2"), makeOptional("dailybuild"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailyfail"))));
}

TEST(RequirementTest, AnyIntersects) {
  Any lhs;
  EXPECT_EQ(*lhs.intersect(lhs), lhs);
  
  {
    AtLeast rhs(ArbiterSemanticVersion(1, 2, 3));
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }
  
  {
    CompatibleWith rhs(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessAllowVersionZeroPatches);
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }
  
  {
    Exactly rhs(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")));
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }
}

TEST(RequirementTest, AtLeastIntersects) {
  AtLeast lhs(ArbiterSemanticVersion(1, 2, 3));
  EXPECT_EQ(*lhs.intersect(lhs), lhs);

  {
    AtLeast rhs(ArbiterSemanticVersion(1, 3, 0, makeOptional("alpha.1")));
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }

  {
    CompatibleWith rhs(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessAllowVersionZeroPatches);
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }

  {
    CompatibleWith rhs(ArbiterSemanticVersion(1, 2, 0), ArbiterRequirementStrictnessStrict);
    EXPECT_EQ(*lhs.intersect(rhs), CompatibleWith(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessStrict));
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }

  {
    CompatibleWith rhs(ArbiterSemanticVersion(0, 1, 0), ArbiterRequirementStrictnessAllowVersionZeroPatches);
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }

  {
    Exactly rhs(ArbiterSemanticVersion(1, 2, 4, makeOptional("alpha.1")));
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }

  {
    Exactly rhs(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")));
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }
}

TEST(RequirementTest, CompatibleWithIntersects) {
  CompatibleWith lhs(ArbiterSemanticVersion(0, 2, 3), ArbiterRequirementStrictnessAllowVersionZeroPatches);
  EXPECT_EQ(*lhs.intersect(lhs), lhs);

  {
    CompatibleWith rhs(ArbiterSemanticVersion(0, 2, 5), ArbiterRequirementStrictnessAllowVersionZeroPatches);
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }

  {
    CompatibleWith rhs(ArbiterSemanticVersion(0, 3, 0), ArbiterRequirementStrictnessAllowVersionZeroPatches);
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }

  {
    CompatibleWith rhs(ArbiterSemanticVersion(0, 2, 3), ArbiterRequirementStrictnessStrict);
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }

  {
    CompatibleWith rhs(ArbiterSemanticVersion(0, 2, 2), ArbiterRequirementStrictnessStrict);
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }

  {
    Exactly rhs(ArbiterSemanticVersion(0, 2, 3));
    EXPECT_EQ(*lhs.intersect(rhs), rhs);
    EXPECT_EQ(*lhs.intersect(rhs), *rhs.intersect(lhs));
  }

  {
    Exactly rhs(ArbiterSemanticVersion(0, 2, 3, makeOptional("alpha.1")));
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }
}

TEST(RequirementTest, ExactlyIntersects) {
  Exactly lhs(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("1")));
  EXPECT_EQ(*lhs.intersect(lhs), lhs);

  {
    Exactly rhs(ArbiterSemanticVersion(1, 2, 3));
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }

  {
    Exactly rhs(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")));
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }

  {
    Exactly rhs(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("2")));
    EXPECT_EQ(lhs.intersect(rhs), nullptr);
    EXPECT_EQ(rhs.intersect(lhs), nullptr);
  }
}
