// import { SemanticVersion } from "../";
const { SemanticVersion } = require("../");

test("Version", () => {
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
