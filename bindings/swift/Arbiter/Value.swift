/**
 * Represents an arbitrary value that can be associated with Arbiter data types
 * and functionality.
 *
 * For example, ProjectIdentifiers are defined using a value conforming to this
 * protocol.
 */
public protocol ArbiterValue : AnyObject, Comparable, Hashable
{}

extension ArbiterValue {
  private typealias UnmanagedValue = Unmanaged<Self>

  /**
   * Creates a C structure representation of this value.
   *
   * This creates an unbalanced retain on the object, with the expectation that
   * the structure will be passed to an Arbiter API which invokes the
   * destructor. Failure to hand over the structure to Arbiter will result in
   * a memory leak.
   */
  public func toUserValue () -> ArbiterUserValue
  {
    let wrapper = UserValueWrapper(
      data: UnsafeMutablePointer(Unmanaged.passRetained(self).toOpaque()),
      equalTo: { first, second in
        let unmanagedFirst = UnmanagedValue.fromOpaque(COpaquePointer(first))
        let unmanagedSecond = UnmanagedValue.fromOpaque(COpaquePointer(second))
        return unmanagedFirst.takeUnretainedValue() == unmanagedSecond.takeUnretainedValue()
      },
      lessThan: { first, second in
        let unmanagedFirst = UnmanagedValue.fromOpaque(COpaquePointer(first))
        let unmanagedSecond = UnmanagedValue.fromOpaque(COpaquePointer(second))
        return unmanagedFirst.takeUnretainedValue() < unmanagedSecond.takeUnretainedValue()
      },
      hash: { ptr in
        let unmanaged = UnmanagedValue.fromOpaque(COpaquePointer(ptr))
        return unmanaged.takeUnretainedValue().hashValue
      },
      createDescription: { ptr in
        let unmanaged = UnmanagedValue.fromOpaque(COpaquePointer(ptr))
        let str = String(unmanaged.takeUnretainedValue())

        return str.withCString { constStr in
          return strdup(constStr)
        }
      },
      destructor: { ptr in
        UnmanagedValue.fromOpaque(COpaquePointer(ptr)).release()
      })

    return ArbiterUserValue(
      data: UnsafeMutablePointer(Unmanaged.passRetained(wrapper).toOpaque()),
      equalTo: { first, second in
        let unmanagedFirst = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(first))
        let unmanagedSecond = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(second))
        return unmanagedFirst.takeUnretainedValue() == unmanagedSecond.takeUnretainedValue()
      },
      lessThan: { first, second in
        let unmanagedFirst = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(first))
        let unmanagedSecond = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(second))
        return unmanagedFirst.takeUnretainedValue() < unmanagedSecond.takeUnretainedValue()
      },
      hash: { ptr in
        let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeUnretainedValue()
        return wrapper.hash(UnsafePointer(wrapper.data))
      },
      createDescription: { ptr in
        let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeUnretainedValue()
        return wrapper.createDescription(UnsafePointer(wrapper.data))
      },
      destructor: { ptr in
        let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeRetainedValue()
        wrapper.destructor(wrapper.data)
      })
  }

  /**
   * Reads a value of this type from a C pointer.
   *
   * This method cannot perform any typechecking, so the pointed-to data must
   * correspond to an ArbiterUserValue created from toUserValue() on this type.
   */
  public static func fromUserValue (ptr: UnsafePointer<Void>) -> Self
  {
    let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeUnretainedValue()
    return UnmanagedValue.fromOpaque(COpaquePointer(wrapper.data)).takeUnretainedValue()
  }
}

/**
 * Trampoline object that allows us to bundle real Swift closures into an
 * ArbiterUserValue.
 *
 * Normally, Swift closures cannot be used as C function pointer callbacks,
 * because function pointers are not allowed to close over any state. We solve
 * this by using this trampoline object as the "context" pointer to the
 * callbacks, and then unwrapping the real Swift objects we care about from
 * there.
 */
private class UserValueWrapper : Comparable
{
  typealias Comparator = (UnsafePointer<Void>, UnsafePointer<Void>) -> Bool

  let data: UnsafeMutablePointer<Void>
  let equalTo: Comparator
  let lessThan: Comparator
  let hash: (UnsafePointer<Void> -> Int)
  let createDescription: (UnsafePointer<Void> -> UnsafeMutablePointer<CChar>)
  let destructor: (UnsafeMutablePointer<Void> -> Void)

  init (data: UnsafeMutablePointer<Void>, equalTo: Comparator, lessThan: Comparator, hash: (UnsafePointer<Void> -> Int), createDescription: (UnsafePointer<Void> -> UnsafeMutablePointer<CChar>), destructor: (UnsafeMutablePointer<Void> -> Void))
  {
    self.data = data
    self.equalTo = equalTo
    self.lessThan = lessThan
    self.hash = hash
    self.createDescription = createDescription
    self.destructor = destructor
  }
}

private func == (lhs: UserValueWrapper, rhs: UserValueWrapper) -> Bool
{
  return lhs.equalTo(UnsafePointer(lhs.data), UnsafePointer(rhs.data))
}

private func < (lhs: UserValueWrapper, rhs: UserValueWrapper) -> Bool
{
  return lhs.lessThan(UnsafePointer(lhs.data), UnsafePointer(rhs.data))
}
