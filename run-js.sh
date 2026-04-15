#!/bin/bash
# Detect an available JavaScript interpreter and use it to run combinators.js.
# Tries, in order of preference: node, nodejs, deno, bun, qjs, d8, jsc.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JS_FILE="${SCRIPT_DIR}/combinators.js"

if [ ! -f "${JS_FILE}" ]; then
    echo "run-js.sh: ${JS_FILE} not found" >&2
    exit 1
fi

run_with() {
    local cmd="$1"
    shift
    echo "Running combinators.js with ${cmd}..."
    exec "${cmd}" "$@" "${JS_FILE}"
}

if command -v node >/dev/null 2>&1; then
    run_with node
elif command -v nodejs >/dev/null 2>&1; then
    run_with nodejs
elif command -v deno >/dev/null 2>&1; then
    run_with deno run
elif command -v bun >/dev/null 2>&1; then
    run_with bun run
elif command -v qjs >/dev/null 2>&1; then
    run_with qjs
elif command -v d8 >/dev/null 2>&1; then
    run_with d8
elif command -v jsc >/dev/null 2>&1; then
    run_with jsc
else
    echo "run-js.sh: no JavaScript interpreter found" >&2
    echo "  tried: node, nodejs, deno, bun, qjs, d8, jsc" >&2
    exit 127
fi
