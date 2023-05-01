
# csnap

[![Build status](https://ci.appveyor.com/api/projects/status/find8sqtr41m8akm?svg=true)](https://ci.appveyor.com/project/strandfield/csnap)

`csnap` is a libclang-based command-line utility to create snapshots of C++ programs.

Snapshots produced by csnap are saved as a SQLite database, making them easily usable 
in other programs.
csnap can also export a snapshot as HTML so that it can be viewed online.

## Continuous integration (CI)

**AppVeyor**

[![Build status](https://ci.appveyor.com/api/projects/status/find8sqtr41m8akm?svg=true)](https://ci.appveyor.com/project/strandfield/csnap)

The AppVeyor build is configured to take a snapshot of csnap and export 
it as HTML.
See `appveyor.yml` and the [build artifacts](https://ci.appveyor.com/project/strandfield/csnap/build/artifacts).
