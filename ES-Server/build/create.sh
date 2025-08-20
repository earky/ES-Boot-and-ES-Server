#!/bin/bash

# 检查参数是否存在
if [ $# -ne 1 ]; then
    echo "错误：请提供1个字符串参数"
    exit 1
fi

input_str="$1"
str_len=${#input_str}

# 验证字符串长度是否为8个字符
if [ "$str_len" -ne 8 ]; then
    echo "错误：字符串长度必须为8个字符（当前长度：$str_len）"
    exit 1
fi

# 创建目录（如果不存在）
target_dir="./$input_str"
if [ ! -d "$target_dir" ]; then
    mkdir -p "$target_dir"
    echo "创建目录：$target_dir"
else
    echo "目录已存在：$target_dir"
fi

# 复制文件并验证结果
copy_success=0
files=("v0.0.bin" "info.json")

for file in "${files[@]}"; do
    if [ -f "./$file" ]; then
        cp "./$file" "$target_dir"
        echo "已复制：$file → $target_dir/"
        ((copy_success++))
    else
        echo "警告：未找到文件 - $file"
    fi
done

# 操作结果摘要
if [ $copy_success -eq ${#files[@]} ]; then
    echo -e "\n✅ 操作完成！所有文件已复制到 $target_dir/"
else
    echo -e "\n⚠️ 部分操作完成！成功复制 $copy_success/${#files[@]} 个文件"
    echo "请检查缺失文件"
fi