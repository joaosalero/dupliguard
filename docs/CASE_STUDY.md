# Reviewing duplicate candidates on a local filesystem

DupliGuard answers a question that precedes manual cleanup: which paths currently appear to contain the same bytes? Files and names stay on the machine performing the scan. Reports include absolute paths, so publishing a report from a personal directory would reveal its names.

## A controlled scan

The [reproduction script](../examples/scan_demo.py) writes four temporary files. Two contain `same content\n`. A third contains `different---\n`, also 13 bytes. The fourth contains a single byte. The script invokes the compiled CLI and checks its JSON against an independent SHA-256 calculation, then checks the names and bytes of all four inputs.

On an Ubuntu 24.04 runner with GNU C/C++ 13.3.0, the CLI enumerated four files totaling 40 bytes. Only `copy.txt` and `original.txt` appeared in the group. The report estimated 13 logical bytes of redundancy. The same-size control shows why size alone would have produced a false match. The runtime field is an integer millisecond count from one tiny fixture and has no value as a throughput benchmark.

[Captured JSON](evidence/scan-report.json) · [Native run, step “Verify synthetic scan and print the native JSON”](https://github.com/joaosalero/dupliguard/actions/runs/45231591804/job/137892264017)

To reproduce on a Unix-like system with the project's Make toolchain and Python 3:

```sh
make
python3 examples/scan_demo.py ./build/dupliguard
```

The temporary absolute path and elapsed time vary between runs.

## Why group before hashing?

The C++ scanner indexes regular-file paths by logical size. It hashes only buckets containing multiple eligible paths. It reads in 64 KiB blocks and passes each block to the C hash core. The CLI sorts group members before serialization. This avoids content reads for unique sizes while keeping the hashing state small. The path index, grouped results and rendered report still grow with the scan. The optional Java desktop starts the CLI as a child process and retains its complete text output.

[Scanner and reports](../cli/src/scanner.cpp) · [Hash core](../core/src/sha256.c) · [Desktop integration](../desktop/src/main/java/io/dupliguard/DesktopApp.java)

## Output integrity

Explicit report files use exclusive creation. Existing output paths are refused, including symlinks. The CLI checks write and close results. For stdout, it now flushes before checking stream state, so a buffered write failure can set a failing exit code. The Linux regression directs a small report to a full device and checks that the process fails. Shell redirection opens its destination before the CLI starts, so use `--output` when existing paths must be protected. After a failed write, an incomplete report may remain.

[CLI output handling](../cli/src/main.cpp) · [Integration tests](../tests/test_cli.sh)

## Operational limits and next decision

Scan a stable directory under the operator's control. The scanner skips symlink entries seen below the chosen root. The root may itself resolve through a symlink. Size collection and later opens are separate filesystem operations, so the result has no snapshot consistency. For a live dataset where consistency matters, scan a filesystem snapshot. The fixture above used a private temporary directory.

Hard links are counted by path and may already share their underlying storage. Matching hashes are candidates for review. The scanner omits an independent byte-by-byte confirmation and performs no deletion. A zero warning count does not establish complete coverage because permission-denied directories can be skipped. On Unix, non-UTF-8 bytes in filenames can also make a JSON report unreadable to a strict UTF-8 consumer; this demonstration uses ASCII names.

If cleanup decisions ever depend on the reported space, first add per-platform file identity and a revalidation step immediately before any user-approved action. That would need separate tests for hard links, replacements and permissions on each supported filesystem.
