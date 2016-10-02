public class ResolverBase: CObject {
  private override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    super.init(pointer, shouldCopy: shouldCopy)
  }

  private func createDependencyList (project: COpaquePointer, selectedVersion: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
  {
    preconditionFailure("Must be overridden by subclasses")
  }

  private func createAvailableVersionsList (project: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
  {
    preconditionFailure("Must be overridden by subclasses")
  }

  private func createSelectedVersionForMetadata (metadata: UnsafePointer<Void>) -> COpaquePointer
  {
    preconditionFailure("Must be overridden by subclasses")
  }
}

public final class Resolver<ProjectValue: ArbiterValue, VersionMetadata: ArbiterValue> : ResolverBase
{
  public typealias ListDependencies = (Resolver, ProjectIdentifier<ProjectValue>, SelectedVersion<VersionMetadata>) throws -> DependencyList<ProjectValue>
  public typealias ListAvailableVersions = (Resolver, ProjectIdentifier<ProjectValue>) throws -> SelectedVersionList<VersionMetadata>
  public typealias SelectedVersionForMetadata = VersionMetadata -> SelectedVersion<VersionMetadata>?

  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    // This omits initialization of the callback variables, which should be okay
    // as it is impossible to set up new behaviors for this ArbiterResolver
    // pointer.
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (initialGraph: ResolvedDependencyGraph<ProjectValue, VersionMetadata>?, dependenciesToResolve: DependencyList<ProjectValue>, listDependencies: ListDependencies, listAvailableVersions: ListAvailableVersions, selectedVersionForMetadata: SelectedVersionForMetadata? = nil)
  {
    // This has to be initialized in two steps, because we need a reference to
    // `self` in order to create the ArbiterResolver pointer.
    self.init(nil, shouldCopy: false)

    self.listDependencies = listDependencies
    self.listAvailableVersions = listAvailableVersions
    self.selectedVersionForMetadata = selectedVersionForMetadata

    let behaviors = ArbiterResolverBehaviors(
      createDependencyList: createDependencyListBehavior,
      createAvailableVersionsList: createAvailableVersionsListBehavior,
      createSelectedVersionForMetadata: createSelectedVersionForMetadataBehavior)

    _pointer = ArbiterCreateResolver(behaviors, initialGraph?.pointer ?? nil, dependenciesToResolve.pointer, toUserContext(self))
  }

  private override func createDependencyList (project: COpaquePointer, selectedVersion: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
  {
    do {
      let deps = try listDependencies(self, ProjectIdentifier<ProjectValue>(project, shouldCopy: false), SelectedVersion<VersionMetadata>(selectedVersion, shouldCopy: false))
      return deps.takeOwnership()
    } catch let ex {
      String(ex).withCString { str in
        error.memory = strdup(str)
      }

      return nil
    }
  }

  private override func createAvailableVersionsList (project: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
  {
    do {
      let versions = try listAvailableVersions(self, ProjectIdentifier<ProjectValue>(project, shouldCopy: false))
      return versions.takeOwnership()
    } catch let ex {
      String(ex).withCString { str in
        error.memory = strdup(str)
      }

      return nil
    }
  }

  private override func createSelectedVersionForMetadata (metadata: UnsafePointer<Void>) -> COpaquePointer
  {
    guard let selectedVersionForMetadata = selectedVersionForMetadata else {
      return nil
    }

    if let version = selectedVersionForMetadata(VersionMetadata.fromUserValue(metadata)) {
      return version.takeOwnership()
    } else {
      return nil
    }
  }

  private var listDependencies: ListDependencies! = nil
  private var listAvailableVersions: ListAvailableVersions! = nil
  private var selectedVersionForMetadata: SelectedVersionForMetadata? = nil
}

private func createDependencyListBehavior (resolver: COpaquePointer, project: COpaquePointer, selectedVersion: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
{
  let resolver: ResolverBase = fromUserContext(ArbiterResolverContext(resolver))
  return resolver.createDependencyList(project, selectedVersion: selectedVersion, error: error)
}

private func createAvailableVersionsListBehavior (resolver: COpaquePointer, project: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
{
  let resolver: ResolverBase = fromUserContext(ArbiterResolverContext(resolver))
  return resolver.createAvailableVersionsList(project, error: error)
}

private func createSelectedVersionForMetadataBehavior (resolver: COpaquePointer, metadata: UnsafePointer<Void>) -> COpaquePointer
{
  let resolver: ResolverBase = fromUserContext(ArbiterResolverContext(resolver))
  return resolver.createSelectedVersionForMetadata(metadata)
}
