/**
 * A collection which lazily loads a backing collection.
 */
public class OnDemandCollection<C: CollectionType> : CollectionType
{
  public typealias Collection = C
  public typealias Generator = Collection.Generator
  public typealias SubSequence = Collection.SubSequence
  public typealias Index = Collection.Index

  init(startIndex: Index, endIndex: Index, load: () -> Collection)
  {
    self._load = load
    self.startIndex = startIndex
    self.endIndex = endIndex
  }

  private let _load: () -> Collection
  private var _collection: Collection?

  public let startIndex: Index
  public let endIndex: Index

  public var collection: Collection
  {
    if let c = _collection {
      return c
    } else {
      let c = _load()
      _collection = c
      return c
    }
  }

  public subscript(bounds: Range<Index>) -> SubSequence
  {
    return collection[bounds]
  }

  public subscript(position: Index) -> Generator.Element
  {
    return collection[position]
  }

  public func generate() -> Generator
  {
    return collection.generate()
  }
}
