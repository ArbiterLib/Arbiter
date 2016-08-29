public class SemanticVersion
{
  init (major: Int, minor: Int, patch: Int, prereleaseVersion: String? = nil, buildMetadata: String? = nil) {
    // Makes the compiler happy
    _semanticVersion = nil

    maybeWithCString(prereleaseVersion) { prereleaseVersion in
      maybeWithCString(buildMetadata) { buildMetadata in
        _semanticVersion = UnsafeMutablePointer(ArbiterCreateSemanticVersion(UInt32(major), UInt32(minor), UInt32(patch), prereleaseVersion, buildMetadata))
      }
    }
  }

  deinit {
    _semanticVersion.destroy()
    _semanticVersion.dealloc(1)
  }

  private let _semanticVersion: UnsafeMutablePointer<Void>
}

private func maybeWithCString<Result> (str: String?, @noescape f: UnsafePointer<Int8> throws -> Result) rethrows -> Result
{
  if let str = str {
    return try f(str)
  } else {
    return try f(nil)
  }
}
