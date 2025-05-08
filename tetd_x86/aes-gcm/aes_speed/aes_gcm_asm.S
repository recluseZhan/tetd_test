//
// aes_gcm_asm.S
// void aes_gcm_encrypt(const u8 *pt, const u8 key[16], const u8 iv[12],
//                      size_t len, u8 *ct, u8 tag[16]);
//   rdi = pt, rsi = key, rdx = iv, rcx = len, r8 = ct, r9 = tag
//
#include <linux/linkage.h>
#include <asm/ibt.h>

    .text
    .align 16
SYM_FUNC_START(aes_gcm_encrypt)
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $288, %rsp           # 256B for roundkeys + 32B for H,J0,E0,A,L

    # 保存 len 到 r12
    movq    %rcx, %r12

    # 1) AES‑128 key expansion into stack[0..160]
    movdqu  (%rsi), %xmm0        # load K
    movdqa  %xmm0, %xmm1
    movdqu  %xmm0, 0(%rsp)       # roundkey[0]

    aeskeygenassist $0x01, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1, 16(%rsp)      # roundkey[1]

    aeskeygenassist $0x02, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1, 32(%rsp)      # roundkey[2]

    aeskeygenassist $0x04, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1, 48(%rsp)      # roundkey[3]

    aeskeygenassist $0x08, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1, 64(%rsp)      # roundkey[4]

    aeskeygenassist $0x10, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1, 80(%rsp)      # roundkey[5]

    aeskeygenassist $0x20, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1, 96(%rsp)      # roundkey[6]

    aeskeygenassist $0x40, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1,112(%rsp)      # roundkey[7]

    aeskeygenassist $0x80, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1,128(%rsp)      # roundkey[8]

    aeskeygenassist $0x1B, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1,144(%rsp)      # roundkey[9]

    aeskeygenassist $0x36, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3; pslldq $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pslldq  $4, %xmm3; pxor %xmm3, %xmm1
    pxor    %xmm2, %xmm1
    movdqu  %xmm1,160(%rsp)      # roundkey[10]

    # 2) H = AES(K, 0^128)
    pxor    %xmm0, %xmm0
    pxor    0(%rsp), %xmm0
    aesenc  16(%rsp), %xmm0
    aesenc  32(%rsp), %xmm0
    aesenc  48(%rsp), %xmm0
    aesenc  64(%rsp), %xmm0
    aesenc  80(%rsp), %xmm0
    aesenc  96(%rsp), %xmm0
    aesenc 112(%rsp), %xmm0
    aesenc 128(%rsp), %xmm0
    aesenc 144(%rsp), %xmm0
    aesenclast 160(%rsp), %xmm0
    movdqa  %xmm0,176(%rsp)      # store H

    # 3) J0 = IV||1
    movq    (%rdx), %rax
    movq    %rax,192(%rsp)
    movl    8(%rdx),%ecx
    movl    %ecx,200(%rsp)
    movl    $1,204(%rsp)

    # 4) E0 = AES(K, J0)
    movdqu 192(%rsp),%xmm0
    pxor    0(%rsp),%xmm0
    aesenc  16(%rsp),%xmm0
    aesenc  32(%rsp),%xmm0
    aesenc  48(%rsp),%xmm0
    aesenc  64(%rsp),%xmm0
    aesenc  80(%rsp),%xmm0
    aesenc  96(%rsp),%xmm0
    aesenc 112(%rsp),%xmm0
    aesenc 128(%rsp),%xmm0
    aesenc 144(%rsp),%xmm0
    aesenclast 160(%rsp),%xmm0
    movdqa  %xmm0,208(%rsp)      # store E0

    # 5) A = 0 (GHASH acc)
    pxor    %xmm0,%xmm0
    movdqa  %xmm0,224(%rsp)

    # 6) CTR‑AES + GHASH loop
    xorq    %rax,%rax            # offset i
.loop:
    cmpq    $0,%rcx
    je      .finish

    # Ji = J0 + (i/16)
    movl    204(%rsp),%ecx
    addl    %eax,%ecx
    movl    %ecx,204(%rsp)

    # E(K,Ji)
    movdqu 192(%rsp),%xmm0
    pxor    0(%rsp),%xmm0
    aesenc  16(%rsp),%xmm0
    aesenc  32(%rsp),%xmm0
    aesenc  48(%rsp),%xmm0
    aesenc  64(%rsp),%xmm0
    aesenc  80(%rsp),%xmm0
    aesenc  96(%rsp),%xmm0
    aesenc 112(%rsp),%xmm0
    aesenc 128(%rsp),%xmm0
    aesenc 144(%rsp),%xmm0
    aesenclast 160(%rsp),%xmm0

    # 加密 16B 文本
    movdqu  (%rdi,%rax),%xmm1
    pxor    %xmm0,%xmm1
    movdqu  %xmm1,(%r8,%rax)

    # GHASH: U = (A⊕C)·H
    movdqa  224(%rsp),%xmm2
    pxor    %xmm1,%xmm2
    movdqa  176(%rsp),%xmm3

    movdqa  %xmm2,%xmm4; pclmulqdq $0x00,%xmm3,%xmm4
    movdqa  %xmm2,%xmm5; pclmulqdq $0x11,%xmm3,%xmm5
    movdqa  %xmm2,%xmm6; pclmulqdq $0x10,%xmm3,%xmm6
    movdqa  %xmm2,%xmm7; pclmulqdq $0x01,%xmm3,%xmm7

    pxor    %xmm6,%xmm5
    movdqa  %xmm5,%xmm6
    psrldq  $8,%xmm5
    pslldq  $8,%xmm6
    pxor    %xmm6,%xmm4
    pxor    %xmm5,%xmm7
    pxor    %xmm7,%xmm4

    # reduce
    movdqa  %xmm4,%xmm5
    psrldq  $8,%xmm4
    pslldq  $8,%xmm5
    pxor    %xmm5,%xmm4

    movdqa  %xmm4,224(%rsp)

    addq    $16,%rax
    subq    $16,%rcx
    jmp     .loop

.finish:
    # 7) L = [0||bits(len)] at 240
    movq    $0,240(%rsp)
    movq    %r12,%rax
    shlq    $3,%rax
    movq    %rax,248(%rsp)

    # 8) 最后 GHASH: V = (A⊕L)·H
    movdqa  224(%rsp),%xmm2
    movdqu  240(%rsp),%xmm1
    pxor    %xmm1,%xmm2
    movdqa  176(%rsp),%xmm3

    movdqa  %xmm2,%xmm4; pclmulqdq $0x00,%xmm3,%xmm4
    movdqa  %xmm2,%xmm5; pclmulqdq $0x11,%xmm3,%xmm5
    movdqa  %xmm2,%xmm6; pclmulqdq $0x10,%xmm3,%xmm6
    movdqa  %xmm2,%xmm7; pclmulqdq $0x01,%xmm3,%xmm7

    pxor    %xmm6,%xmm5
    movdqa  %xmm5,%xmm6
    psrldq  $8,%xmm5
    pslldq  $8,%xmm6
    pxor    %xmm6,%xmm4
    pxor    %xmm5,%xmm7
    pxor    %xmm7,%xmm4

    movdqa  %xmm4,%xmm5
    psrldq  $8,%xmm4
    pslldq  $8,%xmm5
    pxor    %xmm5,%xmm4

    # 9) Tag = E0 ⊕ V
    movdqu 208(%rsp),%xmm1
    pxor    %xmm4,%xmm1
    movdqu  %xmm1,(%r9)

    # epilogue
    movq    %rbp,%rsp
    popq    %rbp
    RET
SYM_FUNC_END(aes_gcm_encrypt)

