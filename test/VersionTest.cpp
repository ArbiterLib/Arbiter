#include "gtest/gtest.h"

#include "Version-inl.h"

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
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2-alpha.1").value(), ArbiterSemanticVersion(1, 0, 2, Optional<std::string>("alpha.1")));
}

TEST(VersionTest, ParsesBuildMetadata) {
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2+dailybuild").value(), ArbiterSemanticVersion(1, 0, 2, Optional<std::string>(), Optional<std::string>("dailybuild")));
}

TEST(VersionTest, ParsesPrereleaseVersionAndBuildMetadata) {
  EXPECT_EQ(ArbiterSemanticVersion::fromString("1.0.2-alpha.1+dailybuild").value(), ArbiterSemanticVersion(1, 0, 2, Optional<std::string>("alpha.1"), Optional<std::string>("dailybuild")));
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
