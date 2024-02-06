#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
from typing import Any


def clangtidy(*args: Any, **kwargs: Any) -> None:
	subprocess.run(["clang-tidy", *args], check=True, **kwargs)


def main(*args: Any) -> int:
	try:
		clangtidy("--version", capture_output=True)
	except:
		print("ERROR: clang-tidy is required")
		return 1

	scriptdir: Path = Path(os.path.dirname(__file__))
	projectdir: Path = scriptdir / ".." / ".."

	sourcedir: Path = projectdir / "src"
	ccdbs: list[Path] = list(projectdir.rglob("compile_commands.json"))

	if len(ccdbs) == 0:
		print("ERROR: Could not find compile_commands.json")
		return 1

	if len(ccdbs) > 1:
		print("ERROR: Found multiple compile_commands.json")
		return 1

	error: bool = False
	ccdb: Path = ccdbs[0]

	for file in sourcedir.rglob("*"):
		if not file.is_file():
			continue

		if file.suffix not in [".c", ".cpp"]:
			continue

		print("Checking %s ..." % file.resolve())

		try:
			clangtidy(*args, "-p", ccdb.parent, file)
		except KeyboardInterrupt:
			return 1
		except:
			error = True

	if error:
		return 1

	return 0


if __name__ == "__main__":
	sys.exit(main(*sys.argv[1:]))
