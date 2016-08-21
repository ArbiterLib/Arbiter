#include "Requirement.h"

#include "gtest/gtest.h"

using namespace Arbiter;

TEST(RequirementTest, Any) {
  Requirement::Any req;
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, Requirement::Any());
  EXPECT_NE(req, Requirement::AtLeast(ArbiterSemanticVersion(1, 2, 3)));

  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"))));
}

TEST(RequirementTest, AtLeast) {
  Requirement::AtLeast req(ArbiterSemanticVersion(1, 2, 3));
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, Requirement::AtLeast(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_NE(req, Requirement::AtLeast(ArbiterSemanticVersion(1, 2, 4)));
  EXPECT_NE(req, Requirement::AtLeast(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_NE(req, Requirement::Any());

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
  Requirement::AtLeast req(ArbiterSemanticVersion(0, 0, 1));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 1)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 2)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 1, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
}

TEST(RequirementTest, CompatibleWith) {
  Requirement::CompatibleWith req(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessStrict);
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, Requirement::CompatibleWith(ArbiterSemanticVersion(1, 2, 3), ArbiterRequirementStrictnessStrict));
  EXPECT_NE(req, Requirement::CompatibleWith(ArbiterSemanticVersion(1, 2, 4), ArbiterRequirementStrictnessStrict));
  EXPECT_NE(req, Requirement::CompatibleWith(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")), ArbiterRequirementStrictnessStrict));
  EXPECT_NE(req, Requirement::AtLeast(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_NE(req, Requirement::Any());

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
  Requirement::CompatibleWith req(ArbiterSemanticVersion(0, 2, 3), ArbiterRequirementStrictnessStrict);
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3, makeOptional("alpha.1"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4, makeOptional("alpha.1"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 3, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
}

TEST(RequirementTest, CompatibleWithMajorVersionZeroLoose) {
  Requirement::CompatibleWith req(ArbiterSemanticVersion(0, 2, 3), ArbiterRequirementStrictnessAllowVersionZeroPatches);
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 0, 0)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 3, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4)));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(0, 2, 4, makeOptional("alpha.1"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(0, 3, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
}

TEST(RequirementTest, Exactly) {
  Requirement::Exactly req(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")));
  EXPECT_EQ(req, *req.clone());
  EXPECT_EQ(req, Requirement::Exactly(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_NE(req, Requirement::Exactly(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_NE(req, Requirement::Exactly(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_NE(req, Requirement::AtLeast(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_NE(req, Requirement::Any());

  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 0, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(2, 0, 0)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3)));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"))));
  EXPECT_TRUE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 4, makeOptional("alpha.1"), makeOptional("dailybuild"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.2"), makeOptional("dailybuild"))));
  EXPECT_FALSE(req.satisfiedBy(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailyfail"))));
}
