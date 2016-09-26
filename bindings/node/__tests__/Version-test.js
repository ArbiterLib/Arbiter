import { SemanticVersion } from "../";
import jsc, { asciinestring, constant, forall, oneof, uint32 } from "jsverify";

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
      const [major, minor, patch, prereleaseVersion, buildMetadata] = args;
      const version = factory(...args);
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
  describe("when called with new", () => {
    itBehavesLikeASemanticVersion((...args) => new SemanticVersion(...args));
  });

  describe("when called without new", () => {
    itBehavesLikeASemanticVersion(SemanticVersion);
  });
});
