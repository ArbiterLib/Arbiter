# Arbiter

Arbiter is a C library which implements the basic behaviors needed by any decentralized dependency manager or package manager, without being coupled to any particular platform or purpose, so that more specific tools can be built on top.

For example, Arbiter could be used to implement a package manager for:

 * **Development libraries**, similar to [npm](http://npmjs.com), [CocoaPods](https://cocoapods.org), [Cargo](https://github.com/rust-lang/cargo), etc.
 * **Installable tools**, similar to [Homebrew](http://brew.sh)

## Functionality

Functionality that should be provided by Arbiter includes:

 * **Dependency resolution**, without needing any centralized list of available dependencies (similar to the [Carthage](https://github.com/Carthage/Carthage) model)
 * GitHub integration?
 * VCS integration?
 * ???

## License

Arbiter is released under the [MIT license](LICENSE).
