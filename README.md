# DupliGuard

**Privacy-first duplicate file auditor powered by C, C++, and Java.**

DupliGuard finds duplicate candidates with matching sizes and SHA-256 hashes without uploading filenames or
contents. A portable C hashing core does the heavy work, a modern C++ CLI walks
the filesystem and produces machine-readable reports, and a Java Swing desktop
app makes the same engine accessible to non-technical users.

> Safe by design: DupliGuard is read-only. It reports duplicates; it never
> deletes, moves, or modifies scanned files. An explicit `--output` writes a new report and refuses existing paths.

## Why this project exists

Cloud cleanup tools can require users to expose private data. DupliGuard runs
entirely on-device and demonstrates a practical polyglot architecture with a
small trust boundary. The native CLI uses the platform runtime; the optional desktop app requires Java 17+.

## Features

- SHA-256 hashing implemented in portable C
- Efficient two-stage scan: files are grouped by size before hashing
- Recursive C++17 scanner with JSON and human-readable output
- Java 17+ desktop interface with background scanning
- Symlinks are skipped and filesystem errors are isolated per entry
- Deterministic output, automated tests, sanitizer build, and CI
- Linux, macOS, and Windows-compatible source code

## Architecture

| Component | Responsibility | Boundary |
| --- | --- | --- |
| Java 17 Swing desktop | Folder selection and background scan results | Launches the CLI as a child process |
| C++17 CLI | Filesystem traversal, size grouping and report generation | Calls the C hashing API |
| C11 core | Incremental SHA-256 hashing | Fixed-size hash state and caller-provided buffers |

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
--output FILE         Create a new report; existing paths are refused
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

## Illustrative output

The following values describe the output format, not a measured benchmark.

```text
Scanned 8 files (24.1 MB) in 12 ms
Found 2 duplicate groups, 3 redundant copies, 7.8 MB reclaimable

SHA-256  9f86d081884c7d65...  (2 files, 4.0 MB each)
  /home/user/Documents/report-copy.pdf
  /home/user/Downloads/report.pdf
```

## Performance and safety limits

Files are grouped by size before hashing, so unique-size files do not incur content reads.
The hashing loop reads 64 KiB at a time and keeps fixed-size SHA-256 state; it does not
load whole files into memory. The path index and report still grow with the number of
files. The Java desktop captures the complete text report in memory, so its memory use
also grows with report size. No throughput or maximum dataset size has been benchmarked.

Scan a stable directory that you control. Traversal and hashing do not form an atomic
filesystem snapshot: a file may change, disappear, or be replaced between enumeration
and opening. Symlinks observed during traversal are skipped, but this is not protection
against an adversary replacing paths concurrently. Stop writers or use a filesystem
snapshot when consistency matters. A scan is not a sandbox for an untrusted directory.

Matching hashes are not followed by a separate byte comparison. Hard links are not
deduplicated by file identity, and the reclaimable-byte total is an estimate from logical
sizes, not a guarantee of physical disk savings. Review results and verify current
content before any manual cleanup. Reports contain local paths and should be handled
as private data. Warning counts are not proof of complete coverage: the iterator can
silently skip permission-denied directories.

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
