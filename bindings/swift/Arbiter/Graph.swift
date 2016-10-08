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

  public func graphWithNewRoots<S: SequenceType where S.Generator.Element == ProjectIdentifier<ProjectValue>> (projects: S) -> ResolvedDependencyGraph
  {
    var pointers: [COpaquePointer] = []
    for project in projects {
      pointers.append(project.pointer)
    }

    let ptr = pointers.withUnsafeBufferPointer { bufferPtr in
      return ArbiterResolvedDependencyGraphCopyWithNewRoots(self.pointer, bufferPtr.baseAddress, bufferPtr.count)
    }

    return ResolvedDependencyGraph(ptr, shouldCopy: false)
  }

  public func addNode (node: ResolvedDependency<ProjectValue, VersionMetadata>, requirement: Requirement) throws
  {
    var cStr: UnsafeMutablePointer<CChar> = nil

    if (!ArbiterResolvedDependencyGraphAddNode(pointer, node.pointer, requirement.pointer, &cStr)) {
      let string = String(UTF8String: cStr)
      free(cStr)

      throw ArbiterError(message: string)
    }
  }

  public func addEdgeFromDependent (dependent: ProjectIdentifier<ProjectValue>, toDependency dependency: ResolvedDependency<ProjectValue, VersionMetadata>, requirement: Requirement) throws
  {
    var cStr: UnsafeMutablePointer<CChar> = nil

    if (!ArbiterResolvedDependencyGraphAddEdge(pointer, dependent.pointer, dependency.pointer, &cStr)) {
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

public final class ResolvedDependencyInstaller<ProjectValue: ArbiterValue, VersionMetadata: ArbiterValue> : CObject
{
  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (graph: ResolvedDependencyGraph<ProjectValue, VersionMetadata>)
  {
    self.init(ArbiterResolvedDependencyInstallerCreate(graph.pointer), shouldCopy: false)
  }

  public var phases: [OnDemandCollection<[ResolvedDependency<ProjectValue, VersionMetadata>]>]
  {
    let phaseCount = ArbiterResolvedDependencyInstallerPhaseCount(pointer)

    var phases: [OnDemandCollection<[ResolvedDependency<ProjectValue, VersionMetadata>]>] = []
    phases.reserveCapacity(phaseCount)

    for phaseIndex in 0..<phaseCount {
      let count = ArbiterResolvedDependencyInstallerCountInPhase(pointer, phaseIndex)

      phases.append(OnDemandCollection(startIndex: 0, endIndex: count) {
        let buffer = UnsafeMutablePointer<COpaquePointer>.alloc(count)
        ArbiterResolvedDependencyInstallerGetAllInPhase(self.pointer, phaseIndex, buffer)

        let array = UnsafeBufferPointer(start: buffer, count: count).map { ptr in
          return ResolvedDependency<ProjectValue, VersionMetadata>(ptr)
        }

        buffer.destroy(count)
        buffer.dealloc(count)

        return array
      })
    }

    return phases
  }
}
