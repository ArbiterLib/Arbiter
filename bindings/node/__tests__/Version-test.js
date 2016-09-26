import { SemanticVersion } from "../";

test("Version without prerelease version or build metadata", () => {
  const major = 1;
  const minor = 0;
  const patch = 2;
  const prereleaseVersion = null;
  const buildMetadata = null;
  const version = new SemanticVersion(major, minor, patch, prereleaseVersion, buildMetadata);
  expect(version.getMajorVersion()).toBe(major);
  expect(version.getMinorVersion()).toBe(minor);
  expect(version.getPatchVersion()).toBe(patch);
  expect(version.getPrereleaseVersion()).toBe(prereleaseVersion);
  expect(version.getBuildMetadata()).toBe(buildMetadata);
});

test("Version with prerelease version and build metadata", () => {
  const major = 1;
  const minor = 2;
  const patch = 3;
  const prereleaseVersion = "alpha.0";
  const buildMetadata = "dailybuild";
  const version = new SemanticVersion(major, minor, patch, prereleaseVersion, buildMetadata);
  expect(version.getMajorVersion()).toBe(major);
  expect(version.getMinorVersion()).toBe(minor);
  expect(version.getPatchVersion()).toBe(patch);
  expect(version.getPrereleaseVersion()).toBe(prereleaseVersion);
  expect(version.getBuildMetadata()).toBe(buildMetadata);
});
