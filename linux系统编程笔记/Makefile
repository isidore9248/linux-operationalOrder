# project_root/          # 项目根目录
# ├── src/               # 存放源代码
# │   ├── main.c
# │   ├── module1.c
# │   └── module2.c
# ├── include/           # 存放头文件
# │   ├── module1.h
# │   └── module2.h
# ├── build/             # 编译输出目录（Makefile运行后生成）
# │   ├── main.o
# │   ├── module1.o
# │   └── module2.o
# ├── Makefile           # Makefile文件
# └── test               # 生成的可执行文件


TARGET = test
GNUTOOL = gcc

# 编译选项，包含 include 目录
CFLAGS = -Wall -g -Iinclude

# 获取目录下的所有.c文件，并包含 main.c
SRC = main.c $(wildcard src/*.c)

# 将所有.c文件编译成指定目录下的.o文件
BUILD_DIR = build
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))

# $^ 表示所有依赖
# $@ 表示目标
$(TARGET): $(OBJS)
	$(GNUTOOL) $^ -o $@

# 创建.o文件所在的目录，并编译.o文件
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)/src
	mkdir -p $(dir $@)        # 创建.o文件的输出目录
	$(GNUTOOL) -c $< -o $@ $(CFLAGS)

# 创建 build 目录和子目录
$(BUILD_DIR)/src:
	mkdir -p $(BUILD_DIR)/src

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
