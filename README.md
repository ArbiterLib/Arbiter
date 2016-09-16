# Arbiter [![Build Status](https://travis-ci.org/jspahrsummers/Arbiter.svg?branch=master)](https://travis-ci.org/jspahrsummers/Arbiter)

Arbiter is a cross-platform C library<sup>1</sup> which implements the baseline functionality that should be expected of any dependency manager or package manager, without being coupled to any particular use case, so that many more specific tools can be built on top.

In other words, **Arbiter does not prescribe any one user experience**—it just tries to solve those backend concerns which are common to all dependency managers.

_<sup>1</sup> Note that Arbiter is actually implemented in C++14, but currently only exposes a plain C API to minimize surface area and maximize interoperability._

## Motivation

Arbiter’s main contributors originally built [Carthage](https://github.com/Carthage/Carthage), a dependency manager for iOS and macOS frameworks. Carthage introduced some novel ideas which aren’t commonly found in other dependency managers, like a focus on [decentralization](#lazy-decentralized-dependency-resolution). Unfortunately, Carthage is fairly coupled to Apple’s development tools and process.

Arbiter was conceived, in part, to **generalize the best of Carthage’s features** for use by _any_ dependency manager, on a variety of platforms.

There are also some baseline features that we believe any dependency manager should support in order to be taken seriously, including:

* **Dependency “freezing.”** Different tools have different names for this concept, but the basic idea is the same: once dependencies have been resolved, _all projects and the versions selected for each_ should be saved to disk so that anyone else collaborating on the parent project can reproduce the dependency checkouts exactly.
* **Automatic conflict resolution.** If two projects in the dependency graph specify mutually exclusive requirements for a shared dependency, the dependency resolver should still be able to back up (e.g., to different versions of the two projects) and try another configuration that might result in success.
* **Safe uninstallation.** The tool should understand when uninstalling one package would break another, and surface this information to the user.

Since there are many dependency managers and package managers which do not meet all of the above criteria, a generic library like Arbiter could be used to fill in the gaps.

## Functionality

Some major features of Arbiter include:

### Compliance with Semantic Versioning

[Semantic Versioning](http://semver.org), or SemVer, is a specification for what software version numbers _mean_, and how they should be used to convey compatibility (and the lack thereof).

Arbiter implements SemVer and incorporates it into its [dependency resolution algorithm](#lazy-decentralized-dependency-resolution), so that complex versioning and compatibility logic does not have to be reinvented from scratch for each new tool.

### Lazy, decentralized dependency resolution

Most package managers require a centralized server which has knowledge of all packages and versions in the system.

However, Arbiter resolves individual dependencies _on demand_, allowing them to be loaded from anywhere—even different places for different versions! This doesn’t preclude using a centralized server, but means that it is not a requirement.

### Parallelizable dependency installation

Arbiter does not itself determine what “installing” a package means, but can provide information to the package manager about the installation process.

Specifically, Arbiter understands when one package must be installed before another, and conversely when certain packages have no implicit relationship to each other. The package manager can use this information to download and install multiple packages concurrently, potentially reducing wait times for the end user.

### … and more to come

For a full list of planned features, check out our [backlog](https://github.com/jspahrsummers/Arbiter/issues?q=is%3Aopen+is%3Aissue+label%3Aenhancement+sort%3Acreated-desc). If you’d be interested in making any of these a reality, please consider [contributing](CONTRIBUTING.md)!

## Documentation

The Arbiter API is extensively documented in header comments, from which we periodically generate [Doxygen pages](http://jspahrsummers.com/Arbiter/). For the public C API, look at headers under [`include/arbiter/`](include/arbiter/) in the [file list of the documentation](http://jspahrsummers.com/Arbiter/files.html).

## Examples

This repository contains not-production-strength [examples](examples/) for demonstrating how the Arbiter API can be used to build different functionality.

To compile all included examples, run `make`.

For more information about individual examples, see the README in each folder. Of course, there are almost certainly other possible uses that we assuredly haven’t thought of or implemented, so this shouldn’t be taken as an exhaustive showcase!

## Bindings

Because the functionality of Arbiter is exposed in a C interface, it’s easy to build bindings into other languages. Currently, Arbiter already has [Swift bindings](bindings/swift/), with more planned!

To compile all included bindings, run `make bindings`.

If you’d like to implement your own bindings, please let us know about them [in a GitHub issue](https://github.com/jspahrsummers/Arbiter/issues/new), and we can include a link here in the README.

## License

Arbiter is released under the [MIT license](LICENSE.md).

I am providing code in this repository to you under an open source license. Because this is my personal repository, the license you receive to my code is from me and not from my employer (Facebook).
