#!/usr/bin/env bash

set -euo pipefail

COMPILE_COMMANDS="$(find . -name 'compile_commands.json')"

if [ "$COMPILE_COMMANDS" = "" ]; then
	echo "Could not find compile_commands.json, exiting."
	exit 1
fi

if [ "$(echo "$COMPILE_COMMANDS" | wc -l)" -lt "1" ]; then
	echo "Found multiple compile_commands.json, exiting."
	exit 1
fi

COMPILE_COMMANDS="$(echo "$COMPILE_COMMANDS" | head -n 1)"
BUILDDIR="$(dirname "$COMPILE_COMMANDS")"

ERROR="0"
FILES="$(find src -name '*.c' -or -name '*.cpp')"

while read -d $'\n' file; do
	echo "Checking $file..."

	if ! clang-tidy "$@" -p "$BUILDDIR" "$file"; then
		ERROR="1"
	fi
done <<< "$FILES"

if [ "$ERROR" = "1" ]; then
	exit 1
fi
