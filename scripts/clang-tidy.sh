#!/bin/bash

set -euo pipefail

# Find compile_commands.json
COMPILE_COMMANDS="$(find . -name 'compile_commands.json')"

if [ "$(echo "$COMPILE_COMMANDS" | wc -l)" = "0" ]; then
	echo "Could not find compile_commands.json, exiting."
	exit 1
fi

if [ "$(echo "$COMPILE_COMMANDS" | wc -l)" -lt "1" ]; then
	echo "Found multiple compile_commands.json, exiting."
	exit 1
fi

# clang-tidy expects the directory of compile_commands.json
COMPILE_COMMANDS="$(echo "$COMPILE_COMMANDS" | head -n 1)"
BUILDDIR="$(dirname "$COMPILE_COMMANDS")"

# Find C and C++ files
find src -name '*.c' -or -name '*.cpp' -print0 | while read -d $'\0' file; do
	echo "Checking $file..."
	clang-tidy -p "$BUILDDIR" "$file"
done
