#include <linux/linkage.h>
#include <asm/ibt.h>

.text

// Key expansion helper
SYM_FUNC_START(key_combine)
    pshufd $0xff, %xmm1, %xmm1
    shufps $0x10, %xmm0, %xmm2
    pxor %xmm2, %xmm0
    shufps $0x8C, %xmm0, %xmm2
    pxor %xmm2, %xmm0
    pxor %xmm1, %xmm0
    RET
SYM_FUNC_END(key_combine)

SYM_FUNC_START(aes_encrypt_128)
    movdqu (%rdi), %xmm0          // Load input
    movdqu (%rdx), %xmm5          // Load original key
    pxor   %xmm2, %xmm2           // Clear xmm2

    // Key expansion
    aeskeygenassist $1, %xmm5, %xmm1
    call key_combine
    movaps %xmm0, %xmm6

    aeskeygenassist $2, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm7

    aeskeygenassist $4, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm8

    aeskeygenassist $8, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm9

    aeskeygenassist $16, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm10

    aeskeygenassist $32, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm11

    aeskeygenassist $64, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm12

    aeskeygenassist $128, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm13

    aeskeygenassist $27, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm14

    aeskeygenassist $54, %xmm0, %xmm1
    call key_combine
    movaps %xmm0, %xmm15

    // Encrypt
    pxor       %xmm5,  %xmm0
    aesenc     %xmm6,  %xmm0
    aesenc     %xmm7,  %xmm0
    aesenc     %xmm8,  %xmm0
    aesenc     %xmm9,  %xmm0
    aesenc     %xmm10, %xmm0
    aesenc     %xmm11, %xmm0
    aesenc     %xmm12, %xmm0
    aesenc     %xmm13, %xmm0
    aesenc     %xmm14, %xmm0
    aesenclast %xmm15, %xmm0

    movdqu %xmm0, (%rsi)         // store to output
    RET
SYM_FUNC_END(aes_encrypt_128)

