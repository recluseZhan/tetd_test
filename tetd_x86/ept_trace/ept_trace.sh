#!/usr/bin/env bash
#
# ept_trace.sh — 跟踪所有 KVM EPT 映射（仅 GPA<2GiB），并生成区间分布
# 日志：
#   - ept_map.log : <timestamp>,<gpa>
#
# 运行：sudo ./ept_trace.sh
# 停止：Ctrl-C
#

LOG_RAW="$(pwd)/ept_map.log"
LOG_RANGE="$(pwd)/ept_map2.log"

echo "[*] 原始日志  : $LOG_RAW"
echo "[*] 区间日志  : $LOG_RANGE"
echo "[*] 立即开始跟踪 GPA < 0x80000000 的 EPT 映射事件..."

########################################
# bpftrace 程序：过滤 GPA < 2GiB，打印 timestamp, gpa
########################################
TRACE_PROG='
kprobe:kvm_tdp_mmu_map
{
    $gpa = *(uint64)(arg1);
    if ($gpa < 0x80000000) {
        printf("%lld,0x%lx\n", nsecs, $gpa);
    }
}'


# TRACE_PROG='
# kprobe:kvm_mmu_page_fault
# {
#     $gpa = arg1;
#     if ($gpa < 0x80000000) {
#         printf("%lld,0x%lx\n", nsecs, $gpa);
#     }
# }'


########################################
# 启动 bpftrace 并保存到 ept_map.log
########################################
if bpftrace -h 2>&1 | grep -q "\-o FILE"; then
  sudo bpftrace -o "$LOG_RAW" -e "$TRACE_PROG"
else
  sudo bpftrace -e "$TRACE_PROG" | tee "$LOG_RAW"
fi
########################################

