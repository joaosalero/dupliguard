# Contributing

Thank you for improving DupliGuard.

1. Open an issue for significant behavior or architecture changes.
2. Create a focused branch from `main`.
3. Run `make test` and, when available, `make sanitize`.
4. Keep the scanner read-only; deletion features require a separate threat model.
5. Submit a pull request describing user impact, tests, and security implications.

Use clear commits and avoid adding dependencies when the standard library is
sufficient. C code targets C11, C++ code targets C++17, and Java targets 17+.
