#!/bin/sh
set -eu

if [ $# -ne 1 ]; then
  echo "Usage: $0 <object_or_elf_file>"
  exit 1
fi

FILE=$1

# 用 awk 直接从 readelf -S 的两行输出里提取 Size
total_bytes=$(
  readelf -S "$FILE" | awk '
    # 匹配第一行 “[  3] .text  … Offset”
    /^\s*\[[[:space:]]*[0-9]+[[:space:]]*\]/ {
      # 读入下一行，那行的第1列就是 Size（hex）
      if (getline > 0) {
        # 把十六进制转成十进制累加
        total += strtonum("0x"$1)
      }
    }
    END { print total }
  '
)

# 如果没拿到任何数据，就报错退出
if [ -z "$total_bytes" ] || [ "$total_bytes" -eq 0 ]; then
  echo "Error: parsing failed or no sections found" >&2
  exit 2
fi

# 计算 4KB 页数（向上取整）
pages=$(( (total_bytes + 4095) / 4096 ))

printf "Total section size: %d bytes, %d pages\n" "$total_bytes" "$pages"

