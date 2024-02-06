#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import sys
from pathlib import Path
from pydoc import importfile
from types import ModuleType


def main() -> int:
	scriptdir: Path = Path(os.path.dirname(__file__))
	formatdir: Path = scriptdir / "format"

	scripts: list[Path] = [
		formatdir / "cpp.py",
		formatdir / "meson.py",
		formatdir / "python.py",
	]

	# Don't create __pycache__ directories
	sys.dont_write_bytecode = True

	for script in scripts:
		mod: ModuleType = importfile(str(script))

		ret: int = mod.main()
		if ret != 0:
			return ret

	return 0


if __name__ == "__main__":
	sys.exit(main())
