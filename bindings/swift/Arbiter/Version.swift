public final class SemanticVersion : Equatable, Comparable
{
  public init (major: Int, minor: Int, patch: Int, prereleaseVersion: String? = nil, buildMetadata: String? = nil) {
    // Makes the compiler happy
    _semanticVersion = nil

    maybeWithCString(prereleaseVersion) { prereleaseVersion in
      maybeWithCString(buildMetadata) { buildMetadata in
        _semanticVersion = UnsafeMutablePointer(ArbiterCreateSemanticVersion(UInt32(major), UInt32(minor), UInt32(patch), prereleaseVersion, buildMetadata))
      }
    }
  }

  public init? (fromString str: String) {
    let ptr = ArbiterCreateSemanticVersionFromString(str)
    if ptr == nil {
      return nil
    }

    _semanticVersion = UnsafeMutablePointer(ptr)
  }

  deinit {
    _semanticVersion.destroy()
    _semanticVersion.dealloc(1)
  }

  public var major: Int {
    return Int(ArbiterGetMajorVersion(COpaquePointer(_semanticVersion)))
  }

  public var minor: Int {
    return Int(ArbiterGetMinorVersion(COpaquePointer(_semanticVersion)))
  }

  public var patch: Int {
    return Int(ArbiterGetPatchVersion(COpaquePointer(_semanticVersion)))
  }

  public var prereleaseVersion: String? {
    let str = ArbiterGetPrereleaseVersion(COpaquePointer(_semanticVersion))
    if str == nil {
      return nil
    }

    return String.fromCString(str)
  }

  public var buildMetadata: String? {
    let str = ArbiterGetBuildMetadata(COpaquePointer(_semanticVersion))
    if str == nil {
      return nil
    }

    return String.fromCString(str)
  }

  private let _semanticVersion: UnsafeMutablePointer<Void>
}

public func == (lhs: SemanticVersion, rhs: SemanticVersion) -> Bool
{
  return ArbiterEqualVersions(COpaquePointer(lhs._semanticVersion), COpaquePointer(rhs._semanticVersion))
}

public func < (lhs: SemanticVersion, rhs: SemanticVersion) -> Bool
{
  return ArbiterCompareVersionOrdering(COpaquePointer(lhs._semanticVersion), COpaquePointer(rhs._semanticVersion)) < 0
}

private func maybeWithCString<Result> (str: String?, @noescape f: UnsafePointer<Int8> throws -> Result) rethrows -> Result
{
  if let str = str {
    return try f(str)
  } else {
    return try f(nil)
  }
}
