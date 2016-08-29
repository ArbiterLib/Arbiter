public enum Specifier
{
  case Any
  case AtLeast(SemanticVersion)
  case CompatibleWith(SemanticVersion, ArbiterRequirementStrictness)
  case Exactly(SemanticVersion)
}

public final class Requirement : CObject
{
  public convenience init (_ specifier: Specifier)
  {
    let ptr: COpaquePointer

    switch specifier {
    case .Any:
      ptr = ArbiterCreateRequirementAny()

    case let .AtLeast(version):
      ptr = ArbiterCreateRequirementAtLeast(version.pointer)

    case let .CompatibleWith(version, strictness):
      ptr = ArbiterCreateRequirementCompatibleWith(version.pointer, strictness)

    case let .Exactly(version):
      ptr = ArbiterCreateRequirementExactly(version.pointer)
    }

    self.init(ptr, shouldCopy: false)
  }

  public func satisfiedBy (version: SemanticVersion) -> Bool
  {
    return ArbiterRequirementSatisfiedBy(pointer, version.pointer)
  }
}
