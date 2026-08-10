#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DATA_DIR="$ROOT_DIR/data"
TARGET_MB="${1:-100}"

if ! [[ "$TARGET_MB" =~ ^[0-9]+$ ]] || [[ "$TARGET_MB" -lt 1 ]]; then
  echo "Usage: $0 [size_in_mb]" >&2
  exit 1
fi

mkdir -p "$DATA_DIR/node1" "$DATA_DIR/node2" "$DATA_DIR/node3"

python3 - "$DATA_DIR" "$TARGET_MB" <<'PY'
import os
import sys

data_dir = sys.argv[1]
size_mb = int(sys.argv[2])
size_bytes = size_mb * 1024 * 1024

for node in (1, 2, 3):
    path = os.path.join(data_dir, f"node{node}", "input.csv")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="") as fh:
        fh.write("id,name,country\n")
        record_index = 1
        while True:
            row = f"{record_index},{'node' + str(node) + '_user' + str(record_index)},country{(record_index % 10)}\n"
            fh.write(row)
            record_index += 1
            if fh.tell() >= size_bytes:
                break
    print(f"Generated {path} ({os.path.getsize(path)} bytes)")
PY

echo "Large mock data generated in $DATA_DIR for $TARGET_MB MB per node"
