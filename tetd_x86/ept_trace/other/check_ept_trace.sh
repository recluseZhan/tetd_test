#!/bin/bash

VM_NAME=$1
TRACE_PATH=/sys/kernel/debug/tracing

FUNC="kvm_mmu_page_fault"


if [ -z "$VM_NAME" ]; then
    echo "用法: $0 <虚拟机名称>"
    exit 1
fi

echo "[*] 检查 tracing 支持..."
if [ ! -d "$TRACE_PATH" ]; then
    echo "错误: tracing 文件系统不存在，请检查 debugfs 是否挂载。"
    echo "尝试运行: mount -t debugfs none /sys/kernel/debug"
    exit 1
fi

echo "[*] 清空 ftrace 设置..."
echo nop > $TRACE_PATH/current_tracer
echo > $TRACE_PATH/trace
echo 0 > $TRACE_PATH/tracing_on

echo "[*] 设置追踪函数: $FUNC"
echo function > $TRACE_PATH/current_tracer

if ! grep -qw "$FUNC" $TRACE_PATH/available_filter_functions; then
    echo "错误: 函数 $FUNC 不支持 ftrace。请尝试其他函数，例如:"
    grep kvm_ $TRACE_PATH/available_filter_functions | head -n 10
    exit 1
fi

echo $FUNC > $TRACE_PATH/set_ftrace_filter
echo 1 > $TRACE_PATH/tracing_on

echo "[*] 启动虚拟机: $VM_NAME"
virsh start $VM_NAME
if [ $? -ne 0 ]; then
    echo "虚拟机启动失败。"
    exit 1
fi

echo "[*] 等待 10 秒收集 EPT 映射调用..."
sleep 10

echo 0 > $TRACE_PATH/tracing_on

LOG_FILE=ept_trace_result.log
grep $FUNC $TRACE_PATH/trace > $LOG_FILE

echo "[*] 收集完毕，结果保存在 $LOG_FILE"

COUNT=$(wc -l < $LOG_FILE)
echo "[*] 映射调用次数: $COUNT"

if [ "$COUNT" -eq 0 ]; then
    echo "[y] 未发现 EPT 映射调用，可能已提前全部映射（2GB 全映射）"
else
    echo "[n] 检测到 $COUNT 次映射调用，EPT 为按需（lazy）建立"
fi

