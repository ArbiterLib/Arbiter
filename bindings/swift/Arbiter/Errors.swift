public struct ArbiterError : ErrorType, CustomStringConvertible
{
  internal init(message: String?)
  {
    self.description = message ?? "ArbiterError"
  }

  public let description: String
}
