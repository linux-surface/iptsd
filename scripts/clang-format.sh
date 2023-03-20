#!/usr/bin/env bash

set -euo pipefail

FILES="$(find src -name '*.c' -or -name '*.cpp' -or -name '*.h' -or -name '*.hpp')"

while read -d $'\n' file; do
	MT_BEFORE="$(stat -c %Y "$file")"

	clang-format -i "$file"

	MT_AFTER="$(stat -c %Y "$file")"

	if [ ! "$MT_BEFORE" = "$MT_AFTER" ]; then
		echo "Formatted: $file"
	fi
done <<< "$FILES"
