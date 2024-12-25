# 定义源文件和目标文件
OBJs = test

# 编译选项，使用静态链接
CFLAGS = -Wall -O2

# 默认目标
all: $(OBJs)

# 生成目标程序
test: bignum.o rsa.o test.o
	$(CC) $(CFLAGS) -o $@ $^ -static

# 编译 bignum.o 文件
bignum.o: bignum.c bignum.h
	$(CC) $(CFLAGS) -c bignum.c

# 编译 rsa.o 文件
rsa.o: rsa.c rsa.h
	$(CC) $(CFLAGS) -c rsa.c

# 编译 test.o 文件
test.o: test.c bignum.h rsa.h
	$(CC) $(CFLAGS) -c test.c

# 清理目标
.PHONY: clean run

# 运行程序
run:
	time ./test

# 清理编译文件
clean:
	- rm $(OBJs) *.o

