#!/usr/bin/env bash

set -euo pipefail

OUTPUT="$(python3 ./scripts/format/cpp.py)"

if [ ! "$OUTPUT" = "" ]; then
	echo "$OUTPUT"
	exit 1
fi
