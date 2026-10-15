#!/usr/bin/env bash
set -euo pipefail

kraft build --plat=qemu --arch=x86_64 --no-prompt | cat

