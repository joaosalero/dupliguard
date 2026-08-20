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
