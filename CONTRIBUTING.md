# Contributing to Arbiter

First of all, consider that [package management is awful, and you should quit right now](https://medium.com/@sdboyer/so-you-want-to-write-a-package-manager-4ae9c17d9527#.bc6kkeux5).

If you’ve made it past that post, great! It’s awesome that you’re interested in contributing! Here are a few guidelines that may prevent wasted effort.

### Consider existing tasks

If you’d like to contribute but maybe aren’t sure where to start, see if there are any open issues with the [help wanted](https://github.com/jspahrsummers/Arbiter/issues?q=is%3Aopen+is%3Aissue+label%3A%22help+wanted%22+sort%3Aupdated-desc) tag. These are features or bug fixes that we want to have, but haven’t gotten around to, so the chances of getting your pull request merged are greater!

### Prefer pull requests

If you know exactly how to implement the feature being suggested or fix the bug being reported, please open a pull request instead of an issue. Pull requests are easier than patches or inline code blocks for discussing and merging the changes.

Of course, if you can’t make the change yourself, or want to discuss whether the idea is interesting before diving into implementation, please [open an issue](https://github.com/jspahrsummers/Arbiter/issues/new) after making sure that one isn’t already logged.

### Match the code style

Please try to match the existing code style! This makes reviewing changes easier, and will result in less back-and-forth.

### C in front, C++ in the back

Arbiter exposes a C99 interface, but is implemented using C++14 under the hood. Headers in [`src/`](src/) may contain C++ code, but all headers in [`include/`](include/) should be C only.

When adding new files, please use `#include` guards and `#pragma once` as seen in our other headers, and minimize cross-header includes as much as possible.

## Getting started

After checking out the repository, you should be able to run `make check` to build the library, all examples, and all bindings, then run the test suite.

If for some reason this step fails, please [open an issue](https://github.com/jspahrsummers/Arbiter/issues/new) if one doesn’t already exist.

**Thanks for contributing! :boom::camel:**
