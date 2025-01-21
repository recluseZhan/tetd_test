#include <stdio.h>
#include <stdlib.h>
#include <string.h>
unsigned long long aes_encrypt(unsigned char *input, unsigned char *output, unsigned char *key);

int main() {
    unsigned char key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 
        0x28, 0xae, 0xd2, 0xa6, 
        0xab, 0xf7, 0x32, 0x29, 
        0x1a, 0xc1, 0x30, 0x08
    };
    
    unsigned char input[16] = {
        0x32, 0x43, 0xf6, 0xa8, 
        0x88, 0x5a, 0x30, 0x8d, 
        0x31, 0x31, 0x98, 0xa2, 
        0xe0, 0x37, 0x07, 0x34
    };
    
    unsigned char output[16];
    for(int i=0;i<256;i++){
        aes_encrypt(input, output, key);
    }  

    return 0;
}

__asm__ (
    ".section .data\n"
    ".comm buf, 16, 16\n"
    
    ".section .text\n"
    
    ".global aes_encrypt\n"
    "aes_encrypt:\n"
    "    movdqu (%rdi), %xmm0\n"
    "    movdqu (%rdx), %xmm5\n"
    "    pxor   %xmm2, %xmm2\n"

    "    aeskeygenassist $1, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm6\n"
    "    aeskeygenassist $2, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm7\n"
    "    aeskeygenassist $4, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm8\n"
    "    aeskeygenassist $8, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm9\n"
    "    aeskeygenassist $16, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm10\n"
    "    aeskeygenassist $32, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm11\n"
    "    aeskeygenassist $64, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm12\n"
    "    aeskeygenassist $128, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm13\n"
    "    aeskeygenassist $27, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm14\n"
    "    aeskeygenassist $54, %xmm0, %xmm1\n"
    "    call key_combine\n"
    "    movaps %xmm0, %xmm15\n"

    "encrypt:\n"
    "    pxor       %xmm5,  %xmm0\n"
    "    aesenc     %xmm6,  %xmm0\n"
    "    aesenc     %xmm7,  %xmm0\n"
    "    aesenc     %xmm8,  %xmm0\n"
    "    aesenc     %xmm9,  %xmm0\n"
    "    aesenc     %xmm10, %xmm0\n"
    "    aesenc     %xmm11, %xmm0\n"
    "    aesenc     %xmm12, %xmm0\n"
    "    aesenc     %xmm13, %xmm0\n"
    "    aesenc     %xmm14, %xmm0\n"
    "    aesenclast %xmm15, %xmm0\n"
    
    "    movdqu %xmm0, (%rsi)\n"
    "    ret\n"

    "key_combine:\n"
    "    pshufd $0b11111111, %xmm1, %xmm1\n"
    "    shufps $0b00010000, %xmm0, %xmm2\n"
    "    pxor   %xmm2, %xmm0\n"
    "    shufps $0b10001100, %xmm0, %xmm2\n"
    "    pxor   %xmm2, %xmm0\n"
    "    pxor   %xmm1, %xmm0\n"
    "    ret\n"
);
