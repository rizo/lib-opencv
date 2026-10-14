#!/usr/bin/env bash
set -euo pipefail

cd src
bearuk --lib opencv -r "$PWD" -w
mv Makefile.uk Makefile.bearuk

