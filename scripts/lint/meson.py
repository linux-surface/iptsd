#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
from typing import Any


def muonanalyze(*args: Any, **kwargs: Any) -> None:
	subprocess.run(["muon", "analyze", *args], check=True, **kwargs)


def main() -> int:
	try:
		muonanalyze("-h", capture_output=True)
	except:
		print("ERROR: muon is required")
		return 1

	scriptdir: Path = Path(os.path.dirname(__file__))
	projectdir: Path = scriptdir / ".." / ".."

	muonanalyze("-Werror", "-O", projectdir)
	return 0


if __name__ == "__main__":
	sys.exit(main())
