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

public func toUserValue<T: AnyObject where T: Comparable> (object: T) -> ArbiterUserValue
{
  let wrapper = UserValueWrapper(
    data: UnsafeMutablePointer(Unmanaged.passRetained(object).toOpaque()),
    equalTo: { first, second in
      let unmanagedFirst = Unmanaged<T>.fromOpaque(COpaquePointer(first))
      let unmanagedSecond = Unmanaged<T>.fromOpaque(COpaquePointer(second))
      return unmanagedFirst.takeUnretainedValue() == unmanagedSecond.takeUnretainedValue()
    },
    lessThan: { first, second in
      let unmanagedFirst = Unmanaged<T>.fromOpaque(COpaquePointer(first))
      let unmanagedSecond = Unmanaged<T>.fromOpaque(COpaquePointer(second))
      return unmanagedFirst.takeUnretainedValue() < unmanagedSecond.takeUnretainedValue()
    },
    destructor: { ptr in
      Unmanaged<T>.fromOpaque(COpaquePointer(ptr)).release()
    },
    createDescription: { ptr in
      let unmanaged = Unmanaged<T>.fromOpaque(COpaquePointer(ptr))
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

public func fromUserValue<T: AnyObject where T: Comparable> (ptr: UnsafePointer<Void>) -> T
{
  let wrapper = Unmanaged<UserValueWrapper>.fromOpaque(COpaquePointer(ptr)).takeUnretainedValue()
  return Unmanaged<T>.fromOpaque(COpaquePointer(wrapper.data)).takeUnretainedValue()
}
