public final class SemanticVersion : Equatable, Comparable
{
  public init (_ pointer: COpaquePointer)
  {
    self.pointer = pointer
  }

  public convenience init (major: Int, minor: Int, patch: Int, prereleaseVersion: String? = nil, buildMetadata: String? = nil)
  {
    var semanticVersionPtr: COpaquePointer = nil

    maybeWithCString(prereleaseVersion) { prereleaseVersion in
      maybeWithCString(buildMetadata) { buildMetadata in
        semanticVersionPtr = ArbiterCreateSemanticVersion(UInt32(major), UInt32(minor), UInt32(patch), prereleaseVersion, buildMetadata)
      }
    }

    self.init(semanticVersionPtr)
  }

  public convenience init? (fromString str: String)
  {
    let ptr = ArbiterCreateSemanticVersionFromString(str)
    if ptr == nil {
      return nil
    }

    self.init(ptr)
  }

  deinit
  {
    ArbiterFreeSemanticVersion(pointer)
  }

  public let pointer: COpaquePointer

  public var major: Int
  {
    return Int(ArbiterGetMajorVersion(pointer))
  }

  public var minor: Int
  {
    return Int(ArbiterGetMinorVersion(pointer))
  }

  public var patch: Int
  {
    return Int(ArbiterGetPatchVersion(pointer))
  }

  public var prereleaseVersion: String?
  {
    let str = ArbiterGetPrereleaseVersion(pointer)
    if str == nil {
      return nil
    }

    return String.fromCString(str)
  }

  public var buildMetadata: String?
  {
    let str = ArbiterGetBuildMetadata(pointer)
    if str == nil {
      return nil
    }

    return String.fromCString(str)
  }
}

public func == (lhs: SemanticVersion, rhs: SemanticVersion) -> Bool
{
  return ArbiterEqualVersions(lhs.pointer, rhs.pointer)
}

public func < (lhs: SemanticVersion, rhs: SemanticVersion) -> Bool
{
  return ArbiterCompareVersionOrdering(lhs.pointer, rhs.pointer) < 0
}

public final class SelectedVersion<Metadata: AnyObject where Metadata: Comparable> /* TODO : Equatable */
{
  // TODO
}

private func maybeWithCString<Result> (str: String?, @noescape f: UnsafePointer<Int8> throws -> Result) rethrows -> Result
{
  if let str = str {
    return try f(str)
  } else {
    return try f(nil)
  }
}
