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

public final class Dependency<ProjectValue: ArbiterValue> : CObject
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

public final class DependencyList<ProjectValue: ArbiterValue> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (_ dependencies: [Dependency<ProjectValue>])
  {
    var pointers: [COpaquePointer] = []
    for dependency in dependencies {
      pointers.append(dependency.pointer)
    }

    let ptr = pointers.withUnsafeBufferPointer { bufferPtr in
      return ArbiterCreateDependencyList(bufferPtr.baseAddress, bufferPtr.count)
    }

    self.init(ptr, shouldCopy: false)
  }
}

public final class ResolvedDependency<ProjectValue: ArbiterValue, VersionMetadata: ArbiterValue> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (project: ProjectIdentifier<ProjectValue>, version: SelectedVersion<VersionMetadata>)
  {
    let ptr = ArbiterCreateResolvedDependency(project.pointer, version.pointer)
    self.init(ptr, shouldCopy: false)
  }

  public var project: ProjectIdentifier<ProjectValue>
  {
    return ProjectIdentifier<ProjectValue>(ArbiterResolvedDependencyProject(pointer))
  }

  public var version: SelectedVersion<VersionMetadata>
  {
    return SelectedVersion<VersionMetadata>(ArbiterResolvedDependencyVersion(pointer))
  }
}

public final class ResolvedDependencyGraph<ProjectValue: ArbiterValue, VersionMetadata: ArbiterValue> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }
	
  public var count: Int {
	return ArbiterResolvedDependencyGraphCount(pointer)
  }
	
  public var depth: Int {
	return ArbiterResolvedDependencyGraphDepth(pointer)
  }
	
  public func countAtDepth(depthIndex: Int) -> Int {
	return ArbiterResolvedDependencyGraphCountAtDepth(pointer, depthIndex)
  }
	
  public func getAll(buffer: UnsafeMutablePointer<COpaquePointer>) {
	ArbiterResolvedDependencyGraphGetAll(pointer, buffer)
  }
	
  public func getAllAtDepth(depthIndex: Int, buffer: UnsafeMutablePointer<COpaquePointer>) {
	ArbiterResolvedDependencyGraphGetAllAtDepth(pointer, depthIndex, buffer)
  }

  // TODO: Memoize the collection and/or generate its elements lazily (so the
  // count can be read without doing all this work).
  public var allDependencies: [ResolvedDependency<ProjectValue, VersionMetadata>] {
    let buffer = UnsafeMutablePointer<COpaquePointer>.alloc(count)
	getAll(buffer)

    let array = UnsafeBufferPointer(start: buffer, count: count).map { ptr in
      return ResolvedDependency<ProjectValue, VersionMetadata>(ptr)
    }

    buffer.destroy(count)
    buffer.dealloc(count)

    return array
  }

  // TODO: Memoize the collection and/or generate its elements lazily (so the
  // count can be read without doing all this work).
  public var depthOrderedDependencies: [[ResolvedDependency<ProjectValue, VersionMetadata>]] {

    var depths: [[ResolvedDependency<ProjectValue, VersionMetadata>]] = []
    depths.reserveCapacity(depth)

    for depthIndex in 0..<depth {
      let count = countAtDepth(depthIndex)
      let buffer = UnsafeMutablePointer<COpaquePointer>.alloc(count)
	  getAllAtDepth(depthIndex, buffer: buffer)

      let array = UnsafeBufferPointer(start: buffer, count: count).map { ptr in
        return ResolvedDependency<ProjectValue, VersionMetadata>(ptr)
      }

      buffer.destroy(count)
      buffer.dealloc(count)

      depths.append(array)
    }

    return depths
  }
}
