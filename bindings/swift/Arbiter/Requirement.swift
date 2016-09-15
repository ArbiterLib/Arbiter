public enum Specifier
{
  case Any
  case AtLeast(SemanticVersion)
  case CompatibleWith(SemanticVersion, ArbiterRequirementStrictness)
  case Exactly(SemanticVersion)
  case Unversioned(ArbiterUserValue)
  // TODO: Custom
  indirect case Compound([Specifier])
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

    case let .Unversioned(metadata):
      ptr = ArbiterCreateRequirementUnversioned(metadata)

    case let .Compound(specifiers):
      let requirements = specifiers.map { s in Requirement(s) }
      let requirementPtrs = requirements.map { $0.pointer }

      ptr = requirementPtrs.withUnsafeBufferPointer { buffer in
        return ArbiterCreateRequirementCompound(buffer.baseAddress, buffer.count)
      }
    }

    self.init(ptr, shouldCopy: false)
  }

  public func satisfiedBy (version: SemanticVersion) -> Bool
  {
    return ArbiterRequirementSatisfiedBy(pointer, version.pointer)
  }
}
