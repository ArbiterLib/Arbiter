#include "gtest/gtest.h"

#include "Version-inl.h"

TEST(VersionTest, Initializes) {
  ArbiterSemanticVersion version(1, 0, 2);
  EXPECT_EQ(version._major, 1);
  EXPECT_EQ(version._minor, 0);
  EXPECT_EQ(version._patch, 2);
  EXPECT_EQ(version._prereleaseVersion.pointer(), nullptr);
  EXPECT_EQ(version._buildMetadata.pointer(), nullptr);
}
