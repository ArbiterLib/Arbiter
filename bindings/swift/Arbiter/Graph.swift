public final class ResolvedDependencyGraph<ProjectValue: ArbiterValue, VersionMetadata: ArbiterValue> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init ()
  {
    self.init(ArbiterResolvedDependencyGraphCreate(), shouldCopy: false)
  }

  public var allDependencies: OnDemandCollection<[ResolvedDependency<ProjectValue, VersionMetadata>]>
  {
    let count = ArbiterResolvedDependencyGraphCount(pointer)

    return OnDemandCollection(startIndex: 0, endIndex: count) {
      let buffer = UnsafeMutablePointer<COpaquePointer>.alloc(count)
      ArbiterResolvedDependencyGraphCopyAll(self.pointer, buffer)

      let array = UnsafeBufferPointer(start: buffer, count: count).map { ptr in
        return ResolvedDependency<ProjectValue, VersionMetadata>(ptr, shouldCopy: false)
      }

      buffer.destroy(count)
      buffer.dealloc(count)

      return array
    }
  }

  public func addRoot (node: ResolvedDependency<ProjectValue, VersionMetadata>, requirement: Requirement) throws
  {
    var cStr: UnsafeMutablePointer<CChar> = nil

    if (!ArbiterResolvedDependencyGraphAddRoot(pointer, node.pointer, requirement.pointer, &cStr)) {
      let string = String(UTF8String: cStr)
      free(cStr)

      throw ArbiterError(message: string)
    }
  }

  public func addEdgeFromDependent (dependent: ProjectIdentifier<ProjectValue>, toDependency dependency: ResolvedDependency<ProjectValue, VersionMetadata>, requirement: Requirement) throws
  {
    var cStr: UnsafeMutablePointer<CChar> = nil

    if (!ArbiterResolvedDependencyGraphAddEdge(pointer, dependent.pointer, dependency.pointer, requirement.pointer, &cStr)) {
      let string = String(UTF8String: cStr)
      free(cStr)

      throw ArbiterError(message: string)
    }
  }

  public func dependenciesOf (project: ProjectIdentifier<ProjectValue>) -> OnDemandCollection<[ProjectIdentifier<ProjectValue>]>
  {
    let count = ArbiterResolvedDependencyGraphCountDependencies(pointer, project.pointer)

    return OnDemandCollection(startIndex: 0, endIndex: count) {
      let buffer = UnsafeMutablePointer<COpaquePointer>.alloc(count)
      ArbiterResolvedDependencyGraphGetAllDependencies(self.pointer, project.pointer, buffer)

      let array = UnsafeBufferPointer(start: buffer, count: count).map { ptr in
        return ProjectIdentifier<ProjectValue>(ptr)
      }

      buffer.destroy(count)
      buffer.dealloc(count)

      return array
    }
  }

  public func versionOf (project: ProjectIdentifier<ProjectValue>) -> SelectedVersion<VersionMetadata>?
  {
    let ptr = ArbiterResolvedDependencyGraphProjectVersion(pointer, project.pointer)
    if ptr == nil {
      return nil
    } else {
      return SelectedVersion(ptr)
    }
  }

  public func requirementOf (project: ProjectIdentifier<ProjectValue>) -> Requirement?
  {
    let ptr = ArbiterResolvedDependencyGraphProjectRequirement(pointer, project.pointer)
    if ptr == nil {
      return nil
    } else {
      return Requirement(ptr)
    }
  }
}
