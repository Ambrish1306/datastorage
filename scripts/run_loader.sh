#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [[ ! -x "$ROOT_DIR/build/datastorage" ]]; then
  echo "Binary not found: $ROOT_DIR/build/datastorage" >&2
  echo "Run ./scripts/build.sh first." >&2
  exit 1
fi

"$ROOT_DIR/build/datastorage"
