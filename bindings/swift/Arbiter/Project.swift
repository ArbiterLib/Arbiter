public final class ProjectIdentifier<Value: ArbiterValue> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (value: Value)
  {
    let ptr = ArbiterCreateProjectIdentifier(value.toUserValue())
    self.init(ptr, shouldCopy: false)
  }

  public var value: Value
  {
    return Value.fromUserValue(ArbiterProjectIdentifierValue(pointer))
  }
}

