#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
from typing import Any


def muonfmt(*args: Any, **kwargs: Any) -> None:
	subprocess.run(["muon", "fmt", *args], check=True, **kwargs)


def main() -> int:
	try:
		muonfmt("-h", capture_output=True)
	except:
		print("ERROR: muon is required")
		return 1

	scriptdir: Path = Path(os.path.dirname(__file__))
	projectdir: Path = scriptdir / ".." / ".."

	cfg: Path = projectdir / ".muon.ini"

	for file in projectdir.rglob("*"):
		if not file.is_file():
			continue

		if file.name not in ["meson.build", "meson_options.txt"]:
			continue

		before: str = file.read_text()

		try:
			muonfmt("-i", "-c", cfg, file)
		except KeyboardInterrupt:
			return 1

		after: str = file.read_text()

		if before != after:
			print("Formatted: %s" % file.resolve())

	return 0


if __name__ == "__main__":
	sys.exit(main())
