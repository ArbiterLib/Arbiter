public final class ProjectIdentifier<Value: AnyObject where Value: Comparable> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (value: Value)
  {
    let ptr = ArbiterCreateProjectIdentifier(toUserValue(value))
    self.init(ptr, shouldCopy: false)
  }

  public var value: Value
  {
    return fromUserValue(ArbiterProjectIdentifierValue(pointer))
  }
}

public final class Dependency<ProjectValue: AnyObject where ProjectValue: Comparable> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (project: ProjectIdentifier<ProjectValue>, requirement: Requirement)
  {
    let ptr = ArbiterCreateDependency(project.pointer, requirement.pointer)
    self.init(ptr, shouldCopy: false)
  }

  public var project: ProjectIdentifier<ProjectValue>
  {
    return ProjectIdentifier<ProjectValue>(ArbiterDependencyProject(pointer))
  }

  public var requirement: Requirement
  {
    return Requirement(ArbiterDependencyRequirement(pointer))
  }
}
