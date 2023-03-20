#!/usr/bin/env bash

set -euo pipefail

exec bash ./scripts/clang-tidy.sh --warnings-as-errors='*'
