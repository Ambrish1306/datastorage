#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [[ $# -ne 0 ]]; then
  echo "Usage: $0" >&2
  exit 1
fi

if [[ ! -f "$ROOT_DIR/config/cluster.ini" ]]; then
  echo "Cluster config not found: $ROOT_DIR/config/cluster.ini" >&2
  exit 1
fi

if [[ ! -f "$ROOT_DIR/config/schema.ini" ]]; then
  echo "Schema config not found: $ROOT_DIR/config/schema.ini" >&2
  exit 1
fi

./build/datastorage --cluster "$ROOT_DIR/config/cluster.ini" --schema "$ROOT_DIR/config/schema.ini"
