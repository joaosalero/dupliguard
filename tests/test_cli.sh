#!/usr/bin/env bash
set -euo pipefail
binary=${1:?binary path required}
fixture=$(mktemp -d)
trap 'rm -rf "$fixture"' EXIT
mkdir -p "$fixture/a" "$fixture/b" "$fixture/.hidden"
printf 'same content\n' > "$fixture/a/original.txt"
cp "$fixture/a/original.txt" "$fixture/b/copy.txt"
printf 'different---\n' > "$fixture/unique.txt"
cp "$fixture/a/original.txt" "$fixture/.hidden/hidden-copy.txt"
"$binary" scan "$fixture" --format json > "$fixture/report.json"
python3 - "$fixture/report.json" <<'PY'
import json, sys
data=json.load(open(sys.argv[1], encoding="utf-8"))
assert data["summary"]["duplicateGroups"] == 1, data
assert data["summary"]["redundantFiles"] == 1, data
assert len(data["groups"][0]["files"]) == 2, data
print("CLI integration: OK")
PY

# Report creation must never overwrite an input or an existing report.
cp "$fixture/a/original.txt" "$fixture/expected.txt"
if "$binary" scan "$fixture" --output "$fixture/a/original.txt"; then
  printf 'Existing output unexpectedly accepted\n' >&2
  exit 1
fi
cmp "$fixture/expected.txt" "$fixture/a/original.txt"
ln -s "$fixture/a/original.txt" "$fixture/output-link"
if "$binary" scan "$fixture" --output "$fixture/output-link"; then
  printf 'Output symlink unexpectedly accepted\n' >&2
  exit 1
fi
cmp "$fixture/expected.txt" "$fixture/a/original.txt"
"$binary" scan "$fixture" --format json --output "$fixture/new-report.json"
python3 -m json.tool "$fixture/new-report.json" > /dev/null

for value in '-1' '+1' ' 1' ''; do
  if "$binary" scan "$fixture" --min-size "$value"; then
    printf 'Invalid minimum size unexpectedly accepted: %s\n' "$value" >&2
    exit 1
  fi
done
printf 'CLI output safety and argument validation: OK\n'

# A small report can fit in stdout's buffer even when the destination is full.
if [[ -c /dev/full ]]; then
  if "$binary" scan "$fixture" --format json >/dev/full; then
    printf 'A failed stdout write unexpectedly returned success\n' >&2
    exit 1
  fi
fi
