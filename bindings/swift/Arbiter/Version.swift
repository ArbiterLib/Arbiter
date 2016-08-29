public final class SemanticVersion : Equatable, Comparable
{
  public init (_ semanticVersionPtr: COpaquePointer)
  {
    pointer = UnsafeMutablePointer(semanticVersionPtr)
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
    pointer.destroy()
    pointer.dealloc(1)
  }

  public let pointer: UnsafeMutablePointer<Void>

  public var major: Int
  {
    return Int(ArbiterGetMajorVersion(COpaquePointer(pointer)))
  }

  public var minor: Int
  {
    return Int(ArbiterGetMinorVersion(COpaquePointer(pointer)))
  }

  public var patch: Int
  {
    return Int(ArbiterGetPatchVersion(COpaquePointer(pointer)))
  }

  public var prereleaseVersion: String?
  {
    let str = ArbiterGetPrereleaseVersion(COpaquePointer(pointer))
    if str == nil {
      return nil
    }

    return String.fromCString(str)
  }

  public var buildMetadata: String?
  {
    let str = ArbiterGetBuildMetadata(COpaquePointer(pointer))
    if str == nil {
      return nil
    }

    return String.fromCString(str)
  }
}

public func == (lhs: SemanticVersion, rhs: SemanticVersion) -> Bool
{
  return ArbiterEqualVersions(COpaquePointer(lhs.pointer), COpaquePointer(rhs.pointer))
}

public func < (lhs: SemanticVersion, rhs: SemanticVersion) -> Bool
{
  return ArbiterCompareVersionOrdering(COpaquePointer(lhs.pointer), COpaquePointer(rhs.pointer)) < 0
}

private func maybeWithCString<Result> (str: String?, @noescape f: UnsafePointer<Int8> throws -> Result) rethrows -> Result
{
  if let str = str {
    return try f(str)
  } else {
    return try f(nil)
  }
}
