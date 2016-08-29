public class SemanticVersion
{
  init() {
    _semanticVersion = UnsafeMutablePointer(ArbiterCreateSemanticVersion(1, 0, 0, nil, nil))
  }

  deinit {
    _semanticVersion.destroy()
    _semanticVersion.dealloc(1)
  }

  private let _semanticVersion: UnsafeMutablePointer<Void>
}
