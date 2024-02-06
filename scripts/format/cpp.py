#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
from typing import Any


def clangformat(*args: Any, **kwargs: Any) -> None:
	subprocess.run(["clang-format", *args], check=True, **kwargs)


def main() -> int:
	try:
		clangformat("--version", capture_output=True)
	except:
		print("ERROR: clang-format is required")
		return 1

	scriptdir: Path = Path(os.path.dirname(__file__))
	projectdir: Path = scriptdir / ".." / ".."

	sourcedir: Path = projectdir / "src"

	for file in sourcedir.rglob("*"):
		if not file.is_file():
			continue

		if file.suffix not in [".c", ".cpp", ".h", ".hpp"]:
			continue

		before: str = file.read_text()

		try:
			clangformat("-i", file)
		except KeyboardInterrupt:
			return 1

		after: str = file.read_text()

		if before != after:
			print("Formatted: %s" % file.resolve())

	return 0


if __name__ == "__main__":
	sys.exit(main())
