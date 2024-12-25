#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 8

// 将单个字符转换为16进制格式
void char_to_hex(unsigned char c, FILE *output_file) {
    fprintf(output_file, "\\x%02x", c);
}

// 将PEM格式字符串转换为16进制格式并输出到文件
void pem_to_hex(const char *pem, FILE *output_file) {
    int i;
    int len = strlen(pem);
    int line_count = 0;
    for (i = 0; i < len; i++) {
        // 每行8个\x
        if (line_count == MAX_LINE_LENGTH) {
            fprintf(output_file, "\"\n\"");
            line_count = 0;
        }
        if (pem[i] != '\n') { // 如果不是换行符，则转换为16进制并输出
            char_to_hex(pem[i], output_file);
            line_count++;
        }
    }
}

int main() {
    FILE *input_file = fopen("a.txt", "r");
    if (input_file == NULL) {
        printf("Error opening input file.\n");
        return 1;
    }

    // 获取文件大小
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    // 为文件内容分配空间
    char *pem = (char *)malloc(file_size + 1);
    if (pem == NULL) {
        printf("Error allocating memory.\n");
        fclose(input_file);
        return 1;
    }

    // 读取文件内容
    fread(pem, 1, file_size, input_file);
    pem[file_size] = '\0'; // 添加字符串结束符
    fclose(input_file);

    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Error opening output file.\n");
        free(pem);
        return 1;
    }

    // 输出前缀
    fprintf(output_file, "\"");

    // 将PEM格式转换为16进制并输出到文件
    pem_to_hex(pem, output_file);

    // 输出后缀
    fprintf(output_file, "\"\n");

    fclose(output_file);
    free(pem);
    return 0;
}

