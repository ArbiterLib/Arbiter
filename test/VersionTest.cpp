#include "Version.h"

#include "gtest/gtest.h"

#include <sstream>
#include <utility>

using namespace Arbiter;

TEST(VersionTest, Initializes) {
  ArbiterSemanticVersion version(1, 0, 2);
  EXPECT_EQ(version._major, 1);
  EXPECT_EQ(version._minor, 0);
  EXPECT_EQ(version._patch, 2);
  EXPECT_EQ(version._prereleaseVersion.pointer(), nullptr);
  EXPECT_EQ(version._buildMetadata.pointer(), nullptr);
}

TEST(VersionTest, ParsesSimpleVersions) {
  EXPECT_EQ(ArbiterSemanticVersion::fromString("0.0.0").value(), ArbiterSemanticVersion(0, 0, 0));
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2").value(), ArbiterSemanticVersion(1, 0, 2));
  EXPECT_EQ(ArbiterSemanticVersion::fromString("12.345.6789").value(), ArbiterSemanticVersion(12, 345, 6789));
}

TEST(VersionTest, ParsesPrereleaseVersion) {
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2-alpha.0").value(), ArbiterSemanticVersion(1, 0, 2, makeOptional("alpha.0")));
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2-alpha.1").value(), ArbiterSemanticVersion(1, 0, 2, makeOptional("alpha.1")));
}

TEST(VersionTest, ParsesBuildMetadata) {
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2+dailybuild").value(), ArbiterSemanticVersion(1, 0, 2, None(), makeOptional("dailybuild")));
}

TEST(VersionTest, ParsesPrereleaseVersionAndBuildMetadata) {
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2-alpha.1+dailybuild").value(), ArbiterSemanticVersion(1, 0, 2, makeOptional("alpha.1"), makeOptional("dailybuild")));
}

TEST(VersionTest, FailsToParseMalformedVersions) {
  EXPECT_EQ(ArbiterSemanticVersion::fromString("0").pointer(), nullptr);
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0").pointer(), nullptr);
  EXPECT_EQ(ArbiterSemanticVersion::fromString("-1.0.0").pointer(), nullptr);
  EXPECT_EQ(ArbiterSemanticVersion::fromString("01.0.0").pointer(), nullptr);
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.0a1").pointer(), nullptr);
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.0-alpha.01").pointer(), nullptr);
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.0-alpha$1").pointer(), nullptr);
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.0+build$1").pointer(), nullptr);
}

TEST(VersionTest, ComparesForEquality) {
  EXPECT_EQ(ArbiterSemanticVersion(0, 0, 0), ArbiterSemanticVersion(0, 0, 0));
  EXPECT_EQ(ArbiterSemanticVersion(1, 2, 3), ArbiterSemanticVersion(1, 2, 3));
  EXPECT_NE(ArbiterSemanticVersion(2, 3, 4), ArbiterSemanticVersion(1, 2, 3));
  EXPECT_NE(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")), ArbiterSemanticVersion(1, 2, 3, None()));
  EXPECT_EQ(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")));
  EXPECT_EQ(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")));
  EXPECT_NE(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")), ArbiterSemanticVersion(1, 2, 3, None(), makeOptional("dailybuild")));
  EXPECT_NE(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), None()));
  EXPECT_NE(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.2"), makeOptional("dailybuild")));
}

TEST(VersionTest, ComparesForPrecedence) {
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3), ArbiterSemanticVersion(1, 2, 4));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3), ArbiterSemanticVersion(1, 3, 0));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3), ArbiterSemanticVersion(2, 0, 0));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3), ArbiterSemanticVersion(1, 2, 10));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")), ArbiterSemanticVersion(1, 2, 3));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.2")));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.2")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.10")));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha")), ArbiterSemanticVersion(1, 2, 3, makeOptional("beta")));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1")));
  EXPECT_LT(ArbiterSemanticVersion(1, 2, 3, makeOptional("1")), ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha")));
}

TEST(VersionTest, ConvertsToString) {
  std::stringstream stream;
  stream << ArbiterSemanticVersion(1, 2, 3, makeOptional("alpha.1"), makeOptional("dailybuild"));
  EXPECT_EQ(stream.str(), "1.2.3-alpha.1+dailybuild");
}
