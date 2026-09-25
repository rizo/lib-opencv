#!/usr/bin/env bash
set -euo pipefail

cd src
kraft build --plat=qemu --arch=x86_64 --no-prompt | cat

