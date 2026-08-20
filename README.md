# DupliGuard

**Privacy-first duplicate file auditor powered by C, C++, and Java.**

DupliGuard finds byte-for-byte duplicate files without uploading filenames or
contents. A portable C hashing core does the heavy work, a modern C++ CLI walks
the filesystem and produces machine-readable reports, and a Java Swing desktop
app makes the same engine accessible to non-technical users.

> Safe by design: DupliGuard is read-only. It reports duplicates; it never
> deletes, moves, or modifies files.

## Why this project exists

Cloud cleanup tools can require users to expose private data. DupliGuard runs
entirely on-device and demonstrates a practical polyglot architecture with a
small trust boundary and no runtime dependencies.

## Features

- SHA-256 hashing implemented in portable C
- Efficient two-stage scan: files are grouped by size before hashing
- Recursive C++17 scanner with JSON and human-readable output
- Java 17+ desktop interface with background scanning
- Symlinks are skipped and filesystem errors are isolated per entry
- Deterministic output, automated tests, sanitizer build, and CI
- Linux, macOS, and Windows-compatible source code

## Architecture

```text
Java Swing desktop ──process boundary──> C++ CLI ──C ABI──> C SHA-256 core
                                            │
                                            └── JSON / text report
```

The process boundary keeps the GUI simple and independently replaceable. The C
API can also be embedded by other languages.

## Quick start

### CLI

Requirements: CMake 3.20+ and a C11/C++17 compiler.

```bash
cmake -S . -B build
cmake --build build
./build/dupliguard scan ~/Downloads
./build/dupliguard scan ~/Pictures --format json --output report.json
```

GNU Make users may simply run `make`.

Useful options:

```text
--format text|json    Report format (default: text)
--output FILE         Write the report to a file
--min-size BYTES      Ignore smaller files (default: 1)
--include-hidden      Include hidden files/directories
--help                Show help
```

Exit code `0` means the scan completed. Unreadable entries are reported as a
warning count but do not abort the scan.

### Desktop app

Requirements: JDK 17+ and a built CLI.

```bash
cmake --build build --target dupliguard_desktop
# or: make java
make run-gui
```

On Windows, point the GUI to `dupliguard.exe` if it is not beside the app.

## Example

```text
Scanned 8 files (24.1 MB) in 12 ms
Found 2 duplicate groups, 3 redundant copies, 7.8 MB reclaimable

SHA-256  9f86d081884c7d65...  (2 files, 4.0 MB each)
  /home/user/Documents/report-copy.pdf
  /home/user/Downloads/report.pdf
```

## Development

```bash
make test          # unit + integration tests
make sanitize      # AddressSanitizer and UndefinedBehaviorSanitizer
make clean
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for the development workflow and
[SECURITY.md](SECURITY.md) for responsible disclosure.

## Roadmap

- Signed release binaries for Linux, macOS, and Windows
- Optional content-defined chunking for near-duplicate detection
- User-reviewed quarantine workflow (never automatic deletion)
- Accessibility and localization improvements for the desktop app

## License

MIT — see [LICENSE](LICENSE).
