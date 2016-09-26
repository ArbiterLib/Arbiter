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
const isEquivalent = ({ arb, val }) => {
  return arb === undefined ? val === null : val === arb;
};

// Allows a custom failure message when comparing boolean values.
const expectToBe = (actual, expected, because) => expect(() => {
  try {
    expect(actual).toBe(expected);
  } catch(error) {
    throw new Error(because);
  }
}).not.toThrow();

const checkGetters = (arbMajor, arbMinor, arbPatch, arbPrereleaseVersion, arbBuildMetadata) => {
  const version = new SemanticVersion(arbMajor, arbMinor, arbPatch, arbPrereleaseVersion, arbBuildMetadata);

  const major = version.getMajorVersion();
  const minor = version.getMinorVersion();
  const patch = version.getPatchVersion();
  const prereleaseVersion = version.getPrereleaseVersion();
  const buildMetadata = version.getBuildMetadata();

  const prereleaseVersionIsEquivalent = isEquivalent({
    arb: arbPrereleaseVersion,
    val: prereleaseVersion
  });

  const buildMetadataIsEquivalent = isEquivalent({
    arb: arbBuildMetadata,
    val: buildMetadata
  });

  expect(major).toBe(arbMajor);
  expect(minor).toBe(arbMinor);
  expect(patch).toBe(arbPatch);
  expectToBe(prereleaseVersionIsEquivalent, true, `${prereleaseVersion} !== ${arbPrereleaseVersion}`);
  expectToBe(buildMetadataIsEquivalent, true, `${buildMetadata} !== ${arbBuildMetadata}`);

  return major === arbMajor
    && minor === arbMinor
    && patch === arbPatch
    && prereleaseVersionIsEquivalent
    && buildMetadataIsEquivalent;
};

test("Version getters return correct values", () => {
  jsc.assert(forall(uint32, uint32, uint32, maybe(asciinestring), maybe(asciinestring), checkGetters));
});
