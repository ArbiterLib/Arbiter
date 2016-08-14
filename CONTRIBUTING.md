# Contributing to Arbiter

Although Arbiter exposes a plain C interface, it is implemented using C++14 under the hood. Headers in [`src/internal/`](src/internal/) or whose filenames end with `-inl.h` may contain C++ code. All other headers should be C only.
