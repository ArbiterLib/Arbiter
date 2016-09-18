public enum Specifier
{
  case Any
  case AtLeast(SemanticVersion)
  case CompatibleWith(SemanticVersion, ArbiterRequirementStrictness)
  case Exactly(SemanticVersion)
  case Unversioned(ArbiterUserValue)
  case Custom(RequirementPredicateBase)
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

    case let .Custom(predicate):
      ptr = ArbiterCreateRequirementCustom({ selectedVersion, context in
        let predicate: RequirementPredicateBase = fromUserContext(context)
        return predicate.satisfiedBy(selectedVersion)
      }, toUserContext(predicate))

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

public class RequirementPredicateBase
{
  private init ()
  {}

  private func satisfiedBy (selectedVersion: COpaquePointer) -> Bool
  {
    preconditionFailure("Must be overridden by subclasses")
  }
}

public final class RequirementPredicate<VersionMetadata: ArbiterValue> : RequirementPredicateBase
{
  public typealias Predicate = SelectedVersion<VersionMetadata> -> Bool

  private let closure: Predicate

  public init (_ closure: Predicate)
  {
    self.closure = closure
  }

  private override func satisfiedBy (selectedVersion: COpaquePointer) -> Bool
  {
    return closure(SelectedVersion<VersionMetadata>(selectedVersion))
  }
}
