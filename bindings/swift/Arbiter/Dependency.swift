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
