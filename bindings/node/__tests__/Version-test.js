import jsc, { asciinestring, constant, forall, oneof, uint32 } from "jsverify";

import {
  createSemanticVersion,
  createSemanticVersionFromString
} from "../";

const maybe = (type) => {
  return oneof([
    type,
    constant(null),
    constant(undefined)
  ]);
};

// Test for arbitrary input values of `undefined`
// but account for their return values being `null`.
const isEquivalent = (actual, expected) => {
  return expected === undefined ? actual === null : actual === expected;
};

// Allows for a custom failure message.
const expectToBe = (assertion) => (actual, expected, description) => {
  return expect(() => {
    try {
      assertion(actual, expected);
    } catch(error) {
      const message = !description ? error.message : `
${description}
  actual:   ${actual}
  expected: ${expected}

${error.message}
`;
      throw new Error(message);
    }
  }).not.toThrow();
};

const expectToBeEqual = expectToBe((actual, expected) => {
  return expect(actual).toBe(expected);
});

const expectToBeEquivalent = expectToBe((actual, expected) => {
  return expect(isEquivalent(actual, expected)).toBe(true);
});

const itBehavesLikeASemanticVersion = (factory) => {
  it("returns correct values from getter methods", () => {
    jsc.assert(forall(uint32, uint32, uint32, maybe(asciinestring), maybe(asciinestring), (...args) => {
      const version = factory(...args);
      const [major, minor, patch, prereleaseVersion, buildMetadata] = args;
      expectToBeEqual(version.getMajorVersion(), major, "Major version");
      expectToBeEqual(version.getMinorVersion() , minor, "Minor version");
      expectToBeEqual(version.getPatchVersion(), patch, "Patch version");
      expectToBeEquivalent(version.getPrereleaseVersion(), prereleaseVersion, "Prerelease version");
      expectToBeEquivalent(version.getBuildMetadata(), buildMetadata, "Build metadata");
      return true;
    }));
  });
};

describe("Version", () => {
  describe("createSemanticVersion", () => {
    itBehavesLikeASemanticVersion((...args) => createSemanticVersion(...args));

    it("throws an error with the incorrect number of arguments", () => {
      const message = "Incorrect number of arguments";
      expect(() => createSemanticVersion()).toThrowError(message);
      expect(() => createSemanticVersion("1")).toThrowError(message);
      expect(() => createSemanticVersion("1", "2")).toThrowError(message);
      expect(() => createSemanticVersion("1", "2", "3")).not.toThrow();
      expect(() => createSemanticVersion("1", "2", "3", "alpha.0")).not.toThrow();
      expect(() => createSemanticVersion("1", "2", "3", "alpha.0", "dailybuild")).not.toThrow();
    });
  });

  describe("createSemanticVersionFromString", () => {
    itBehavesLikeASemanticVersion((...args) => {
      const [major, minor, patch] = args;
      const prereleaseVersion = args[3] ? `-${args[3]}` : "";
      const buildMetadata = args[4] ? `+${args[4]}` : "";
      const version = `${major}.${minor}.${patch}${prereleaseVersion}${buildMetadata}`;
      return createSemanticVersionFromString(version);
    });

    it("throws an error with the incorrect number of arguments", () => {
      const message = "Incorrect number of arguments";
      expect(() => createSemanticVersionFromString()).toThrowError(message);
      expect(() => createSemanticVersionFromString("1.2.3")).not.toThrow();
      expect(() => createSemanticVersionFromString("1.2.3", "alpha.0")).toThrowError(message);
    });
  });
});
