# Security policy

## Supported versions

Security fixes are applied to the latest `main` branch until versioned releases
begin.

## Reporting a vulnerability

Please use GitHub's private vulnerability reporting feature for this repository.
Do not include sensitive file contents, personal paths, or real credentials in a
report. You should receive an acknowledgment within seven days.

## Security model

DupliGuard operates locally, opens files read-only, skips symbolic links, and
does not include deletion functionality. Reports can contain local paths and
should be treated as private. The project makes no network requests.
