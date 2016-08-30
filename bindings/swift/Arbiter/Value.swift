public protocol ArbiterValue : AnyObject, Comparable
{}

extension ArbiterValue {
  private typealias UnmanagedValue = Unmanaged<Self>

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
      destructor: { ptr in
        UnmanagedValue.fromOpaque(COpaquePointer(ptr)).release()
      },
      createDescription: { ptr in
        let unmanaged = UnmanagedValue.fromOpaque(COpaquePointer(ptr))
        let str = String(unmanaged.takeUnretainedValue())

        return str.withCString { constStr in
          return strdup(constStr)
        }
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
      destructor: { ptr in
        let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeRetainedValue()
        wrapper.destructor(wrapper.data)
      },
      createDescription: { ptr in
        let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeUnretainedValue()
        return wrapper.createDescription(UnsafePointer(wrapper.data))
      })
  }

  public static func fromUserValue (ptr: UnsafePointer<Void>) -> Self
  {
    let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeUnretainedValue()
    return UnmanagedValue.fromOpaque(COpaquePointer(wrapper.data)).takeUnretainedValue()
  }
}

// Trampoline object that allows us to bundle real Swift closures into an
// ArbiterUserValue.
private class UserValueWrapper : Comparable
{
  typealias Comparator = (UnsafePointer<Void>, UnsafePointer<Void>) -> Bool

  let data: UnsafeMutablePointer<Void>
  let equalTo: Comparator
  let lessThan: Comparator
  let destructor: (UnsafeMutablePointer<Void> -> Void)
  let createDescription: (UnsafePointer<Void> -> UnsafeMutablePointer<CChar>)

  init (data: UnsafeMutablePointer<Void>, equalTo: Comparator, lessThan: Comparator, destructor: (UnsafeMutablePointer<Void> -> Void), createDescription: (UnsafePointer<Void> -> UnsafeMutablePointer<CChar>))
  {
    self.data = data
    self.equalTo = equalTo
    self.lessThan = lessThan
    self.destructor = destructor
    self.createDescription = createDescription
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
