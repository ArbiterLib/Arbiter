/**
 * Base class for any Arbiter Swift object which is backed by a corresponding
 * C object.
 */
public class CObject : Equatable
{
  /**
   * Initializes an object of this type from a C pointer, optionally copying its
   * contents.
   *
   * This initializer cannot perform any typechecking, so the pointer must
   * correspond to an object of this class.
   *
   * If `shouldCopy` is false, the CObject is assumed to retain ownership over
   * the given pointer, and will free it when the CObject is deinitialized by
   * ARC.
   */
  public init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    if (shouldCopy) {
      self._pointer = COpaquePointer(ArbiterCreateCopy(UnsafePointer(pointer)));
    } else {
      self._pointer = pointer
    }
  }

  /**
   * Steals ownership of the `pointer` from this object, returning it.
   *
   * After invoking this method, `pointer` will be nil. The caller is
   * responsible for eventually freeing the stolen pointer.
   */
  public func takeOwnership () -> COpaquePointer
  {
    precondition(_pointer != nil)

    let result = _pointer
    _pointer = nil

    return result
  }

  deinit
  {
    if (_pointer != nil) {
      ArbiterFree(UnsafeMutablePointer(_pointer))
    }
  }

  /**
   * The backing C pointer for this object, or nil.
   */
  public var pointer: COpaquePointer {
    return _pointer
  }

  /**
   * An internally-visible mutable reference to this object's backing pointer.
   *
   * This can be used for two-step initialization (where the CObject needs to
   * complete initializing before the C pointer is known). It is also used in
   * the implementation of takeOwnership().
   */
  var _pointer: COpaquePointer
}

public func == (lhs: CObject, rhs: CObject) -> Bool
{
  return ArbiterEqual(UnsafePointer(lhs.pointer), UnsafePointer(rhs.pointer))
}
