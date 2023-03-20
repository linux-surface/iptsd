#!/usr/bin/env bash

set -euo pipefail

OUTPUT="$(bash ./scripts/clang-format.sh)"

if [ ! "$OUTPUT" = "" ]; then
	echo "$OUTPUT"
	exit 1
fi
