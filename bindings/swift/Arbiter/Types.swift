public class CObject : Equatable
{
  public init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    if (shouldCopy) {
      self._pointer = COpaquePointer(ArbiterCreateCopy(UnsafePointer(pointer)));
    } else {
      self._pointer = pointer
    }
  }

  public func takeOwnership () -> COpaquePointer
  {
    precondition(shouldFree)
    shouldFree = false
    return pointer
  }

  deinit
  {
    ArbiterFree(UnsafeMutablePointer(pointer))
  }

  public var pointer: COpaquePointer {
    return _pointer
  }

  // Mutable internally, for two-step initialization.
  var _pointer: COpaquePointer

  private var shouldFree = true
}

public func == (lhs: CObject, rhs: CObject) -> Bool
{
  return ArbiterEqual(UnsafePointer(lhs.pointer), UnsafePointer(rhs.pointer))
}
