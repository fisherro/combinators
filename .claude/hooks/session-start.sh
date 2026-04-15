#!/bin/bash
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

if ! command -v chibi-scheme &> /dev/null; then
  apt-get install -y chibi-scheme
fi

if ! command -v g++-14 &> /dev/null; then
  apt-get install -y g++-14
fi

update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 200

if ! command -v node &> /dev/null && ! command -v nodejs &> /dev/null; then
  apt-get install -y nodejs
fi
