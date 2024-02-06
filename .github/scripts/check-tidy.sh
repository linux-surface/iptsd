#!/usr/bin/env bash

set -euo pipefail

exec python3 ./scripts/clang-tidy.py --warnings-as-errors='*'
