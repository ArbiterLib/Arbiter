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
	
	lazy var collection: ResolvedDependencyGraphCollection<ProjectValue, VersionMetadata> = {
		return ResolvedDependencyGraphCollection<ProjectValue, VersionMetadata>(pointer: self.pointer)
	}()
	
  public var count: Int {
		return collection.count
  }
	
	public var depth: Int {
		return collection.depth
	}
	
  public func countAtDepth(depthIndex: Int) -> Int {
		return collection.countAtDepth(depthIndex)
  }

  public var allDependencies: [ResolvedDependency<ProjectValue, VersionMetadata>] {
    return collection.allDependencies
  }

  public var depthOrderedDependencies: [[ResolvedDependency<ProjectValue, VersionMetadata>]] {
		return collection.depthOrderedDependencies
  }
}

/**
* Custom collection type to contain the ResolvedDependencyGraph.
*/

internal final class ResolvedDependencyGraphCollection<ProjectValue: ArbiterValue, VersionMetadata: ArbiterValue>: CollectionType
{
	typealias GraphIndex = Int
	typealias Count = Int
	
	private var pointer: COpaquePointer
	
	private var buffer: UnsafeMutablePointer<COpaquePointer>
	
	private var counts: [Int] = []
	
	init(pointer: COpaquePointer)
	{
		self.pointer = pointer
		let count = ArbiterResolvedDependencyGraphCount(pointer)
		self.buffer = UnsafeMutablePointer<COpaquePointer>.alloc(count)
		// disorder
		ArbiterResolvedDependencyGraphGetAll(pointer, buffer)
	}
	
	deinit
	{
		if (buffer != nil) {
			buffer.destroy(count)
			buffer.dealloc(count)
		}
	}
	
	var startIndex: Int
	{
		return 0
	}
	
	var count: Int
	{
		return ArbiterResolvedDependencyGraphCount(pointer);
	}
	
	var depth: Int
	{
		return ArbiterResolvedDependencyGraphDepth(pointer)
	}
	
	var isEmpty: Bool
	{
		return count == 0
	}
	
	var endIndex: Int
	{
		return count
	}
	
	func generate() -> ResolvedDependencyGraphCountGenerator
	{
		return ResolvedDependencyGraphCountGenerator(buffer, count: count)
	}
	
	subscript (index: GraphIndex) -> Count
	{
		precondition(index < endIndex)
		return counts[index]
	}
	
	func countAtDepth(depth: Int) -> Count
	{
		return counts.filter{$0 >= depth}.count
	}
	
	internal var allDependencies: [ResolvedDependency<ProjectValue, VersionMetadata>]
	{
		// we already call ArbiterResolvedDependencyGraphGetAll during initialization
		let array = UnsafeBufferPointer(start: buffer, count: count).map { ptr in
			return ResolvedDependency<ProjectValue, VersionMetadata>(ptr)
		}
		return array
	}
	
	internal var depthOrderedDependencies: [[ResolvedDependency<ProjectValue, VersionMetadata>]]
	{
		// borrow from original implementation
		var depths: [[ResolvedDependency<ProjectValue, VersionMetadata>]] = []
		let depth = ArbiterResolvedDependencyGraphDepth(pointer)
		depths.reserveCapacity(depth)
		
		for depthIndex in 0..<depth {
			let count = countAtDepth(depthIndex)
			let buffer = UnsafeMutablePointer<COpaquePointer>.alloc(count)
			ArbiterResolvedDependencyGraphGetAllAtDepth(pointer, depthIndex, buffer)
			
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

internal struct ResolvedDependencyGraphCountGenerator: GeneratorType {
	
	private var idx = 0
	private var counts: [Int] = []
	
	init(_ ptr: UnsafeMutablePointer<COpaquePointer>, count: Int)
	{
		for i in 0..<count {
			let depth = ArbiterResolvedDependencyGraphDepth(ptr[i])
			counts.append(depth)
		}
	}
	
	internal mutating func next() -> Int? {
		if idx == counts.endIndex {
			return nil
		} else {
			let count = counts[idx]
			idx += 1
			return count
		}
	}
}
