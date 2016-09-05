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
}

public final class Resolver<ProjectValue: ArbiterValue, VersionMetadata: ArbiterValue> : ResolverBase
{
  public typealias ListDependencies = (Resolver, ProjectIdentifier<ProjectValue>, SelectedVersion<VersionMetadata>) throws -> DependencyList<ProjectValue>
  public typealias ListAvailableVersions = (Resolver, ProjectIdentifier<ProjectValue>) throws -> SelectedVersionList<VersionMetadata>

  public override init (_ pointer: COpaquePointer, shouldCopy: Bool = true)
  {
    // This omits initialization of the callback variables, which should be okay
    // as it is impossible to set up new behaviors for this ArbiterResolver
    // pointer.
    super.init(pointer, shouldCopy: shouldCopy)
  }

  public convenience init (dependencies: DependencyList<ProjectValue>, listDependencies: ListDependencies, listAvailableVersions: ListAvailableVersions)
  {
    // This has to be initialized in two steps, because we need a reference to
    // `self` in order to create the ArbiterResolver pointer.
    self.init(nil, shouldCopy: false)

    let behaviors = ArbiterResolverBehaviors(
      createDependencyList: createDependencyListBehavior,
      createAvailableVersionsList: createAvailableVersionsListBehavior)

    let context = Unmanaged.passUnretained(self).toOpaque()
    _pointer = ArbiterCreateResolver(behaviors, dependencies.pointer, UnsafePointer<Void>(context))
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

  private let listDependencies: ListDependencies! = nil
  private let listAvailableVersions: ListAvailableVersions! = nil
}

private func createDependencyListBehavior (resolver: COpaquePointer, project: COpaquePointer, selectedVersion: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
{
  let context = COpaquePointer(ArbiterResolverContext(resolver))
  return Unmanaged<ResolverBase>.fromOpaque(context).takeUnretainedValue().createDependencyList(project, selectedVersion: selectedVersion, error: error)
}

private func createAvailableVersionsListBehavior (resolver: COpaquePointer, project: COpaquePointer, error: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> COpaquePointer
{
  let context = COpaquePointer(ArbiterResolverContext(resolver))
  return Unmanaged<ResolverBase>.fromOpaque(context).takeUnretainedValue().createAvailableVersionsList(project, error: error)
}
