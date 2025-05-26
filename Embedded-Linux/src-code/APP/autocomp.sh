#!/bin/bash

# 定义编译输出文件路径和目标执行路径
BUILD_OUTPUT="/home/book/vscode_project/APP/GetInput"
EXEC_PATH="/home/book/Desktop/myfolder/wdslinux_exefile/"

# 检查编译文件是否存在
if [ ! -f "$BUILD_OUTPUT" ]; then
  echo "编译文件不存在: $BUILD_OUTPUT"
  exit 1
fi

# 将可执行文件复制到目标路径
echo "复制 $BUILD_OUTPUT 到 $EXEC_PATH"
sudo cp "$BUILD_OUTPUT" "$EXEC_PATH"

# 确保复制成功并设置执行权限
if [ $? -eq 0 ]; then
  echo "复制成功，设置执行权限..."
  sudo chmod +x "$EXEC_PATH/$(basename $BUILD_OUTPUT)"
  echo "操作完成"
else
  echo "复制失败"
  exit 1
fi
