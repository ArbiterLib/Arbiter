public class CObject : Equatable
{
  public init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    if (shouldCopy) {
      self.pointer = COpaquePointer(ArbiterCreateCopy(UnsafePointer(pointer)));
    } else {
      self.pointer = pointer
    }
  }

  deinit
  {
    ArbiterFree(UnsafeMutablePointer(pointer))
  }

  public let pointer: COpaquePointer
}

public func == (lhs: CObject, rhs: CObject) -> Bool
{
  return ArbiterEqual(UnsafePointer(lhs.pointer), UnsafePointer(rhs.pointer))
}
