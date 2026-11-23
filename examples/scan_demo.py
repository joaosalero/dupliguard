"""Run the native CLI against four synthetic files and print its original JSON."""

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def main(binary):
    duplicate = b"same content\n"
    inputs = {
        "copy.txt": duplicate,
        "original.txt": duplicate,
        "same-size.txt": b"different---\n",
        "unique-size.txt": b"x",
    }
    with tempfile.TemporaryDirectory(prefix="dupliguard-demo-") as tmp:
        root = Path(tmp).resolve()
        for name, content in inputs.items():
            (root / name).write_bytes(content)

        result = subprocess.run(
            [binary, "scan", str(root), "--format", "json"], capture_output=True, check=True
        )
        report = json.loads(result.stdout)
        summary = report["summary"]
        expected = {
            "filesScanned": 4,
            "bytesScanned": sum(map(len, inputs.values())),
            "duplicateGroups": 1,
            "redundantFiles": 1,
            "reclaimableBytes": len(duplicate),
            "errors": 0,
        }
        if any(summary[key] != value for key, value in expected.items()):
            raise ValueError("Unexpected summary: " + repr(summary))
        expected_group = {
            "sha256": hashlib.sha256(duplicate).hexdigest(),
            "size": len(duplicate),
            "files": [str(root / name) for name in ("copy.txt", "original.txt")],
        }
        if report["groups"] != [expected_group]:
            raise ValueError("Unexpected duplicate group: " + repr(report["groups"]))
        after = {path.name: path.read_bytes() for path in root.iterdir()}
        if after != inputs:
            raise ValueError("The scan changed the synthetic dataset")
        sys.stdout.buffer.write(result.stdout)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Usage: python3 examples/scan_demo.py PATH_TO_DUPLIGUARD")
    main(str(Path(sys.argv[1]).resolve(strict=True)))
