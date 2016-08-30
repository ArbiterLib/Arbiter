public protocol ArbiterValue : AnyObject, Comparable, Hashable
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
