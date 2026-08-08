#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DATA_DIR="$ROOT_DIR/data"
mkdir -p "$DATA_DIR/node1" "$DATA_DIR/node2" "$DATA_DIR/node3"

cat > "$DATA_DIR/node1/input.csv" <<'EOF'
id,name,country
1,alice,us
2,bob,ca
3,charlie,gb
EOF

cat > "$DATA_DIR/node2/input.csv" <<'EOF'
id,name,country
4,dana,au
5,ed,fr
6,frank,de
EOF

cat > "$DATA_DIR/node3/input.csv" <<'EOF'
id,name,country
7,grace,jp
8,hank,br
9,iris,in
EOF

echo "Sample data generated in $DATA_DIR"
