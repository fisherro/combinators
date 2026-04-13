#!/bin/bash
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

if ! command -v chibi-scheme &> /dev/null; then
  apt-get install -y chibi-scheme
fi
