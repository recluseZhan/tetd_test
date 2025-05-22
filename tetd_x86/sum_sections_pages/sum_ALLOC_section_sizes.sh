#!/bin/sh
set -eu

if [ $# -ne 1 ]; then
  echo "Usage: $0 <object_or_elf_file>"
  exit 1
fi

FILE=$1

total_bytes=$(
  readelf -S "$FILE" | awk '
    # 找到 “[  3] .text … Flags” 行，且该行包含 “A” 标志
    /^\s*\[[[:space:]]*[0-9]+[[:space:]]*\].* A/ {
      # 读 Size 那一行
      if (getline > 0) {
        total += strtonum("0x"$1)
      }
    }
    END { print total }
  '
)

pages=$(( (total_bytes + 4095) / 4096 ))
printf "ALLOC section size: %d bytes, %d pages\n" "$total_bytes" "$pages"

