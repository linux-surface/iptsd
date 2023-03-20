#!/bin/bash

set -euo pipefail

# Find C and C++ files
find src -name '*.c' -or -name '*.cpp' -name '*.h' -or -name '*.hpp' -print0 | while read -d $'\0' file; do
	MT_BEFORE="$(stat -c %Y "$file")"

	clang-format -i "$file"

	MT_AFTER="$(stat -c %Y "$file")"

	if [ ! "$MT_BEFORE" = "$MT_AFTER" ]; then
		echo "Formatted: $file"
	fi
done
