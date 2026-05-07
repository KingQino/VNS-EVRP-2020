#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"
DATA_DIR="${DATA_DIR:-$REPO_ROOT/data}"
BIN_PATH="$BUILD_DIR/VNS-David"

resolve_instance() {
    local raw="$1"
    local instance_name
    instance_name="$(basename "$raw")"

    if [ ! -f "$DATA_DIR/$instance_name" ]; then
        echo "Instance not found in data/: $raw" >&2
        exit 1
    fi

    printf '%s\n' "$instance_name"
}

INSTANCES=()
if [ "$#" -gt 0 ]; then
    for raw_instance in "$@"; do
        INSTANCES+=("$(resolve_instance "$raw_instance")")
    done
else
    shopt -s nullglob
    for instance_path in "$DATA_DIR"/*.evrp; do
        INSTANCES+=("$(basename "$instance_path")")
    done
fi

if [ "${#INSTANCES[@]}" -eq 0 ]; then
    echo "No EVRP instances found in $DATA_DIR" >&2
    exit 1
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

for instance_name in "${INSTANCES[@]}"; do
    echo "Running VNS on $instance_name"
    (
        cd "$BUILD_DIR"
        "./$(basename "$BIN_PATH")" "$instance_name"
    )
done

echo "Results written to $REPO_ROOT/stats/VNS"
