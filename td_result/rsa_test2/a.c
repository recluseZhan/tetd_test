#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char *data;
    size_t length;
} BigInt;

// 创建一个指定长度的 BigInt
BigInt *createBigInt(size_t length) {
    BigInt *bigInt = (BigInt *)malloc(sizeof(BigInt));
    if (bigInt == NULL) {
        fprintf(stderr, "Memory allocation failed for BigInt struct.\n");
        exit(EXIT_FAILURE);
    }
    bigInt->data = (unsigned char *)calloc(length, sizeof(unsigned char));
    if (bigInt->data == NULL) {
        fprintf(stderr, "Memory allocation failed for BigInt data.\n");
        free(bigInt);
        exit(EXIT_FAILURE);
    }
    bigInt->length = length;
    return bigInt;
}

// 释放 BigInt
void freeBigInt(BigInt *bigInt) {
    if (bigInt) {
        if (bigInt->data) {
            free(bigInt->data);
        }
        free(bigInt);
    }
}

// 打印 BigInt 的内容
void printBigInt(BigInt *bigInt) {
    for (size_t i = 0; i < bigInt->length; i++) {
        printf("%02x", bigInt->data[i]);
    }
    printf("\n");
}

// 从 unsigned char 数组初始化 BigInt
void initBigInt(BigInt *bigInt, unsigned char *data, size_t length) {
    printf("Initializing BigInt with data length %zu\n", length);
    
    if (length > bigInt->length) {
        fprintf(stderr, "Data length exceeds BigInt length.\n");
        exit(EXIT_FAILURE);
    }
    
    memset(bigInt->data, 0, bigInt->length);
    memcpy(bigInt->data + (bigInt->length - length), data, length);

    printf("BigInt initialized successfully.\n");
}


// 大数加法：result = a + b
void addBigInt(BigInt *result, BigInt *a, BigInt *b) {
    size_t len = a->length;
    unsigned int carry = 0;
    for (int i = len - 1; i >= 0; i--) {
        unsigned int sum = a->data[i] + b->data[i] + carry;
        result->data[i] = sum & 0xFF;
        carry = sum >> 8;
    }
}

// 大数减法：result = a - b
void subBigInt(BigInt *result, BigInt *a, unsigned char *b) {
    printf("Starting subBigInt...\n");

    int borrow = 0;
    size_t b_length = 1; // 假设 b 是单字节的
    for (size_t i = 0; i < a->length; ++i) {
        int subtrahend = (i < b_length) ? b[i] : 0; // 当 b 超过长度时为 0
        int diff = a->data[i] - subtrahend - borrow;
        if (diff < 0) {
            diff += 256;
            borrow = 1;
        } else {
            borrow = 0;
        }
        if (i < result->length) {
            result->data[i] = (unsigned char)diff;
        }
    }

    printf("Completed subBigInt.\n");
}


// 大数乘法：result = a * b (仅支持小数)
void mulBigInt(BigInt *result, BigInt *a, unsigned char b) {
    printf("Starting mulBigInt...\n");
    memset(result->data, 0, result->length);
    size_t carry = 0;
    for (size_t i = 0; i < a->length; ++i) {
        size_t product = a->data[i] * b + carry;
        if (i < result->length) {
            result->data[i] = product & 0xFF;
        }
        carry = product >> 8;
    }
    
    // 如果 result->length 超出 a->length，确保多余的高位被正确赋值
    for (size_t i = a->length; i < result->length; ++i) {
        result->data[i] = carry & 0xFF;
        carry >>= 8;
    }

    printf("Completed mulBigInt.\n");
}

// 左移BigInt一位（即乘以2）：result = a << 1
void shiftLeftBigInt(BigInt *a, int shift) {
    if (shift <= 0) return;  // 无需移位

    int carry = 0;
    for (size_t i = 0; i < a->length; i++) {
        int new_val = (a->data[i] << shift) | carry;
        a->data[i] = new_val & 0xFF;  // 取低8位
        carry = (new_val >> 8) & 0xFF;  // 高位进位
    }
}


// 大数除法：quotient = a / b, remainder = a % b
void divBigInt(BigInt *quotient, BigInt *remainder, BigInt *a, BigInt *b) {
    // 初始化商和余数
    memset(quotient->data, 0, quotient->length);
    initBigInt(remainder, a->data, a->length);

    // 如果 a < b，则直接返回
    if (compareBigInt(a, b) < 0) {
        // 商为0，余数为a本身
        memset(quotient->data, 0, quotient->length);
        return;
    }

    BigInt *temp = createBigInt(a->length);
    BigInt *one = createBigInt(1);
    one->data[0] = 1;

    // 使用试商法逐位减法来计算商和余数
    for (int i = a->length * 8 - 1; i >= 0; i--) {
        // 左移 remainder，一位一位处理
        shiftLeftBigInt(remainder, 1);
        remainder->data[0] |= (a->data[i / 8] >> (i % 8)) & 1;

        if (compareBigInt(remainder, b) >= 0) {
            subBigInt(remainder, remainder, b);
            quotient->data[i / 8] |= (1 << (i % 8));
        }
    }

    // 释放临时变量
    freeBigInt(temp);
    freeBigInt(one);
}


// 大数模运算：result = a % mod
void modBigInt(BigInt *result, BigInt *a, BigInt *mod) {
    // 检查 a 和 mod 是否有效
    if (compareBigInt(a, mod) < 0) {
        // 如果 a 小于 mod，直接将 a 赋值给 result
        initBigInt(result, a->data, a->length);
        return;
    }

    // 临时变量，用于存储除法结果
    BigInt *quotient = createBigInt(a->length);
    BigInt *remainder = createBigInt(mod->length);

    // 大数除法： a / mod，结果为 quotient，余数为 remainder
    divBigInt(quotient, remainder, a, mod);

    // 将余数 remainder 赋值给 result
    initBigInt(result, remainder->data, remainder->length);

    // 释放临时变量
    freeBigInt(quotient);
    freeBigInt(remainder);
}

// 比较函数：0 = a == b, 1 = a > b, -1 = a < b
int compareBigInt(BigInt *a, BigInt *b) {
    for (size_t i = 0; i < a->length; i++) {
        if (a->data[i] > b->data[i]) return 1;
        if (a->data[i] < b->data[i]) return -1;
    }
    return 0;
}

// 模幂运算：result = base^exp % mod
void modExpBigInt(BigInt *result, BigInt *base, BigInt *exp, BigInt *mod) {
    BigInt *tempBase = createBigInt(base->length);
    BigInt *tempExp = createBigInt(exp->length);
    BigInt *tempResult = createBigInt(result->length);
    
    initBigInt(tempBase, base->data, base->length);
    initBigInt(tempExp, exp->data, exp->length);

    result->data[result->length - 1] = 1;  // 初始 result = 1

    while (!isZero(tempExp)) {
        if (tempExp->data[tempExp->length - 1] & 1) {  // exp 是奇数
            mulBigInt(tempResult, result, tempBase->data[0]);
            modBigInt(result, tempResult, mod);
        }
        mulBigInt(tempBase, tempBase, tempBase->data[0]);
        modBigInt(tempBase, tempBase, mod);
        div2(tempExp);  // exp >>= 1
    }

    freeBigInt(tempBase);
    freeBigInt(tempExp);
    freeBigInt(tempResult);
}

// 检查 BigInt 是否为零
int isZero(BigInt *a) {
    for (size_t i = 0; i < a->length; i++) {
        if (a->data[i] != 0) return 0;
    }
    return 1;
}

// 右移除法：result = a / 2
void div2(BigInt *a) {
    unsigned int carry = 0;
    for (size_t i = 0; i < a->length; i++) {
        unsigned int value = a->data[i];
        a->data[i] = (value >> 1) | carry;
        carry = (value & 1) ? 0x80 : 0;
    }
}

// RSA 签名：signature = message^d mod n
void rsaSign(BigInt *signature, BigInt *message, BigInt *privateExponent, BigInt *n) {
    modExpBigInt(signature, message, privateExponent, n);
}

// RSA 验证：signature^e mod n == message
int rsaVerify(BigInt *signature, BigInt *message, BigInt *publicExponent, BigInt *n) {
    BigInt *calculatedMessage = createBigInt(message->length);
    modExpBigInt(calculatedMessage, signature, publicExponent, n);
    
    int isValid = (compareBigInt(calculatedMessage, message) == 0);
    freeBigInt(calculatedMessage);
    return isValid;
}

// 计算最大公因数（用于密钥生成时的互素判断）
void extendedEuclidean(BigInt *a, BigInt *b, BigInt *x, BigInt *y, BigInt *gcd) {
    if (isZero(b)) {
        initBigInt(x, (unsigned char[]){1}, 1); // x = 1
        initBigInt(y, (unsigned char[]){0}, 1); // y = 0
        initBigInt(gcd, a->data, a->length);
        return;
    }
    
    BigInt *x1 = createBigInt(a->length);
    BigInt *y1 = createBigInt(a->length);
    BigInt *tempGcd = createBigInt(a->length);
    BigInt *temp = createBigInt(a->length);
    
    modBigInt(temp, a, b);
    extendedEuclidean(b, temp, x1, y1, tempGcd);
    
    mulBigInt(temp, x1, (unsigned char[]){1});
    subBigInt(x, y1, temp);
    memcpy(y->data, x1->data, y->length);

    memcpy(gcd->data, tempGcd->data, gcd->length);

    freeBigInt(x1);
    freeBigInt(y1);
    freeBigInt(tempGcd);
    freeBigInt(temp);
}

void modInverse(BigInt *result, BigInt *e, BigInt *phi) {
    BigInt *x = createBigInt(phi->length);
    BigInt *y = createBigInt(phi->length);
    BigInt *gcd = createBigInt(phi->length);

    extendedEuclidean(e, phi, x, y, gcd);

    if (compareBigInt(gcd, (unsigned char[]){1}) == 0) {
        initBigInt(result, x->data, x->length);
        if (compareBigInt(result, (unsigned char[]){0}) < 0) {
            addBigInt(result, result, phi);
        }
    } else {
        printf("No modular inverse exists.\n");
    }

    freeBigInt(x);
    freeBigInt(y);
    freeBigInt(gcd);
}

// RSA 密钥生成函数
void rsaKeyGen(BigInt *p, BigInt *q, BigInt *n, BigInt *e, BigInt *d) {
    BigInt *p1 = createBigInt(p->length);
    BigInt *q1 = createBigInt(q->length);
    BigInt *phi = createBigInt(n->length);

    // 计算 n = p * q
    mulBigInt(n, p, q->data[0]);

    // 计算 phi = (p - 1) * (q - 1)
    subBigInt(p1, p, (unsigned char[]){1});
    subBigInt(q1, q, (unsigned char[]){1});
    mulBigInt(phi, p1, q1->data[0]);

    // 设置公钥指数 e
    initBigInt(e, (unsigned char[]){1, 0, 1}, 3); // 通常 e = 65537

    // 计算私钥指数 d，使得 d * e ≡ 1 (mod phi)
    modInverse(d, e, phi);

    freeBigInt(p1);
    freeBigInt(q1);
    freeBigInt(phi);
}

int main() {
    printf("Starting RSA key generation...\n");

    BigInt *p = createBigInt(32);
    BigInt *q = createBigInt(32);
    BigInt *n = createBigInt(64);
    BigInt *e = createBigInt(32);
    BigInt *d = createBigInt(64);

    printf("Initialized BigInt variables.\n");

    // 示例 p 和 q（可以替换为更大的素数）
    unsigned char pVal[32] = {0x00, 0xd5, 0x43, 0x7f};
    unsigned char qVal[32] = {0x00, 0xc7, 0x29, 0xa1};
    initBigInt(p, pVal, sizeof(pVal));
    initBigInt(q, qVal, sizeof(qVal));

    printf("Assigned values to p and q.\n");

    // 生成密钥对
    rsaKeyGen(p, q, n, e, d);
    printf("Generated RSA keys.\n");

    // 定义固定的 32 字节全 1 明文数据
    unsigned char message[32];
    memset(message, 0xFF, 32);

    BigInt *messageBigInt = createBigInt(32);
    memcpy(messageBigInt->data, message, 32);

    // 生成签名
    BigInt *signature = createBigInt(n->length);
    rsaSign(signature, messageBigInt, d, n);

    printf("Signature: ");
    printBigInt(signature);

    // 验证签名
    int isValid = rsaVerify(signature, messageBigInt, e, n);
    if (isValid) {
        printf("Signature is valid.\n");
    } else {
        printf("Signature is invalid.\n");
    }

    // 清理资源
    freeBigInt(p);
    freeBigInt(q);
    freeBigInt(n);
    freeBigInt(e);
    freeBigInt(d);
    freeBigInt(messageBigInt);
    freeBigInt(signature);

    printf("Finished RSA operations.\n");
    return 0;
}
