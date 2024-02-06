#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
from typing import Any


def yapf(*args: Any, **kwargs: Any) -> None:
	subprocess.run(["yapf", *args], check=True, **kwargs)


def isort(*args: Any, **kwargs: Any) -> None:
	subprocess.run(["isort", *args], check=True, **kwargs)


def main() -> int:
	try:
		yapf("--version", capture_output=True)
	except:
		print("ERROR: yapf is required")
		return 1

	try:
		isort("--version", capture_output=True)
	except:
		print("ERROR: isort is required")
		return 1

	scriptdir: Path = Path(os.path.dirname(__file__))
	projectdir: Path = scriptdir / ".." / ".."

	for file in projectdir.rglob("*.py"):
		if not file.is_file():
			continue

		before: str = file.read_text()

		try:
			isort("-q", file)
		except KeyboardInterrupt:
			return 1

		try:
			yapf("-i", file)
		except KeyboardInterrupt:
			return 1

		after: str = file.read_text()

		if before != after:
			print("Formatted: %s" % file.resolve())

	return 0


if __name__ == "__main__":
	sys.exit(main())
