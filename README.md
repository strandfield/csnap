
# csnap

[![Build status](https://ci.appveyor.com/api/projects/status/find8sqtr41m8akm?svg=true)](https://ci.appveyor.com/project/strandfield/csnap)

`csnap` is a libclang-based command-line utility to create snapshots of C++ programs.

Snapshots produced by csnap are saved as a SQLite database, making them easily usable 
in other programs.
csnap can also export a snapshot as HTML so that it can be viewed online.

## Command-line interface (CLI)

**Creating a snapshot**

Syntax:
```
csnap scan --sln <Visual Studio Sln> --output <Database name> [--overwrite] [--threads <N>]
```

Description: 
Creates a snapshot of a C++ program from a Visual Studio solution.

Options:
- `--sln <Visual Studio Sln>`: specify the path of the Visual Studio solution `.sln` file (required)
- `--output <Database name>`: specify the path of SQLite database (required)
- `--overwrite`: specify that the output database should be overwritten if it already exists (optional)
- `--threads <N>`: specify the number of threads used for parsing the translation units (optional)

Examples: 

Create a snapshot using 4 threads for parsing:
```
csnap scan --sln build/csnap.sln --output snapshot.db --threads 4
```

**Exporting a snapshot as HTML**

Syntax:
```
csnap export --snapshot <Snapshot File> --output <Output directory>
```

Description: 
Exports a snapshot produced by `csnap scan` as HTML.

Options:
- `--snapshot <Snapshot File>`: specify the path of the snapshot (required)
- `--output <Output directory>`: specify the directory in which html files will be written (required)

Warning: csnap will overwrite files in the output directory.

Examples: 

Export a snapshot in `output/html`:
```
csnap export --snapshot snapshot.db --output output/html
```

## Continuous integration (CI)

**AppVeyor**

[![Build status](https://ci.appveyor.com/api/projects/status/find8sqtr41m8akm?svg=true)](https://ci.appveyor.com/project/strandfield/csnap)

The AppVeyor build is configured to take a snapshot of csnap and export 
it as HTML.
See `appveyor.yml` and the [build artifacts](https://ci.appveyor.com/project/strandfield/csnap/build/artifacts).
