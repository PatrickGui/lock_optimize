# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -g -pthread

# 目标文件
TARGET = test

# 源文件
SRC = two_thread_deadlock.c

# 默认目标
all: $(TARGET)

# 编译规则
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# 清理目标
clean:
	rm -f $(TARGET)