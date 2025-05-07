#include <linux/linkage.h>
#include <asm/ibt.h>
.text
SYM_FUNC_START(aes_gcm_encrypt)
    # Prologue: set up stack frame (256-byte local stack for keys and data)
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $256, %rsp

    #----------------------------------------
    # AES-128 Key Expansion (expand 16-byte key from RSI into 11 round keys)
    # Input: %rsi -> pointer to 16-byte key
    # Store round keys at [rbp-192] ... [rbp-32] (total 176 bytes for 11 keys)
    # Round 0 key:
    movdqu  (%rsi), %xmm1             # load original key into XMM1
    movdqa  %xmm1, -192(%rbp)        # store roundkey[0]

    # Generate roundkeys 1..10 with AESKEYGENASSIST + shifts/XORs:contentReference[oaicite:3]{index=3}:
    aeskeygenassist $0x01, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2      # replicate high dword
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # now XMM1 = roundkey[1]
    movdqa  %xmm1, -176(%rbp)        # store roundkey[1]

    aeskeygenassist $0x02, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[2]
    movdqa  %xmm1, -160(%rbp)

    aeskeygenassist $0x04, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[3]
    movdqa  %xmm1, -144(%rbp)

    aeskeygenassist $0x08, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[4]
    movdqa  %xmm1, -128(%rbp)

    aeskeygenassist $0x10, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[5]
    movdqa  %xmm1, -112(%rbp)

    aeskeygenassist $0x20, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[6]
    movdqa  %xmm1, -96(%rbp)

    aeskeygenassist $0x40, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[7]
    movdqa  %xmm1, -80(%rbp)

    aeskeygenassist $0x80, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[8]
    movdqa  %xmm1, -64(%rbp)

    aeskeygenassist $0x1B, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[9]
    movdqa  %xmm1, -48(%rbp)

    aeskeygenassist $0x36, %xmm1, %xmm2
    pshufd  $0xff, %xmm2, %xmm2
    movdqa  %xmm1, %xmm3
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pslldq  $4, %xmm3
    pxor    %xmm3, %xmm1
    pxor    %xmm2, %xmm1             # roundkey[10]
    movdqa  %xmm1, -32(%rbp)         # final round key
    #----------------------------------------

    #----------------------------------------
    # Compute H = AES_K(0^128)  (Hash subkey) and save in XMM2
    pxor    %xmm0, %xmm0            # XMM0 = 0 (plaintext block = 0^128)
    movdqa  -192(%rbp), %xmm1       # roundkey[0]
    pxor    %xmm1, %xmm0            # AddRoundKey
    # AES rounds 1..9:
    movdqa  -176(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -160(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -144(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -128(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -112(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -96(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -80(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -64(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -48(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -32(%rbp),  %xmm1; aesenclast %xmm1, %xmm0  # final round
    movdqa  %xmm0, %xmm2            # save H = AES(K,0) in XMM2
    #----------------------------------------

    #----------------------------------------
    # Prepare J0 = IV||0x00000001
    # (IV is 12 bytes at RDX, append 0x00000001 big-endian)
    movq    (%rdx), %rax           # load first 8 bytes of IV
    movq    %rax, -208(%rbp)       # store to J0[0..7]
    movl    8(%rdx), %ecx          # load next 4 bytes of IV
    movl    %ecx, -200(%rbp)       # store to J0[8..11]
    movl    $1, -196(%rbp)         # store 0x00000001 at J0[12..15]

    # Encrypt J0: AES(K, J0) -> gives E(K,J0) in XMM0
    movdqu  -208(%rbp), %xmm0      # load J0 into XMM0
    movdqa  -192(%rbp), %xmm1
    pxor    %xmm1, %xmm0           # AddRoundKey
    movdqa  -176(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -160(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -144(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -128(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -112(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -96(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -80(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -64(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -48(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -32(%rbp),  %xmm1; aesenclast %xmm1, %xmm0  # E(K,J0)
    movdqa  %xmm0, %xmm9            # save E(K,J0) in XMM9
    #----------------------------------------

    #----------------------------------------
    # Prepare J1 = IV||0x00000002 for first counter block
    movl    $2, -196(%rbp)         # overwrite last dword to 2
    movdqu  -208(%rbp), %xmm0      # load J1 into XMM0
    movdqa  -192(%rbp), %xmm1
    pxor    %xmm1, %xmm0           # AddRoundKey
    movdqa  -176(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -160(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -144(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -128(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -112(%rbp), %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -96(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -80(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -64(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -48(%rbp),  %xmm1; aesenc  %xmm1, %xmm0
    movdqa  -32(%rbp),  %xmm1; aesenclast %xmm1, %xmm0  # E(K,J1)
    #----------------------------------------

    #----------------------------------------
    # Compute ciphertext = plaintext XOR E(K,J1)
    movdqu  (%rdi), %xmm3        # load 16-byte plaintext from RDI
    pxor    %xmm0, %xmm3         # XOR with E(K,J1)
    movdqu  %xmm3, (%rcx)        # store ciphertext at RCX
    # XMM3 now holds ciphertext

    #----------------------------------------
    # GHASH: compute GH = (ciphertext ⋅ H)⋅H ⊕ (lengths) in GF(2^128)
    # (A=0, C= ciphertext).  We use the two-step multiply:
    #   U = C · H; then V = (U ⊕ [0||len]) · H.
    movdqa  %xmm2, %xmm1        # reload H into XMM1
    movdqa  %xmm3, %xmm0        # XMM0 = ciphertext (C)

    # First GF multiply: XMM0*=XMM1 -> result in XMM6 (per Intel example:contentReference[oaicite:4]{index=4})
    movdqa  %xmm0, %xmm3
    pclmulqdq $0x00, %xmm1, %xmm3   # a0*b0
    movdqa  %xmm0, %xmm4
    pclmulqdq $0x10, %xmm1, %xmm4   # a0*b1
    movdqa  %xmm0, %xmm5
    pclmulqdq $0x01, %xmm1, %xmm5   # a1*b0
    movdqa  %xmm0, %xmm6
    pclmulqdq $0x11, %xmm1, %xmm6   # a1*b1
    pxor    %xmm5, %xmm4
    movdqa  %xmm4, %xmm5
    psrldq  $8, %xmm4
    pslldq  $8, %xmm5
    pxor    %xmm5, %xmm3
    pxor    %xmm4, %xmm6
    # Combine partial products: now (XMM6:XMM3) = 256-bit product

    # Reduction step (Algorithm with shifts): fold high 128 bits into low (Intel code):contentReference[oaicite:5]{index=5}
    movdqa  %xmm3, %xmm7
    movdqa  %xmm6, %xmm8
    pslld   $1, %xmm3
    pslld   $1, %xmm6
    psrld   $31, %xmm7
    psrld   $31, %xmm8
    movdqa  %xmm7, %xmm9
    pslldq  $4, %xmm8
    pslldq  $4, %xmm7
    psrldq  $12, %xmm9
    por     %xmm7, %xmm3
    por     %xmm8, %xmm6
    por     %xmm9, %xmm6

    movdqa  %xmm3, %xmm7
    movdqa  %xmm3, %xmm8
    movdqa  %xmm3, %xmm9
    pslld   $31, %xmm7   # <<31
    pslld   $30, %xmm8   # <<30
    pslld   $25, %xmm9   # <<25
    pxor    %xmm8, %xmm7
    pxor    %xmm9, %xmm7
    movdqa  %xmm7, %xmm8
    pslldq  $12, %xmm7
    psrldq  $4, %xmm8
    pxor    %xmm7, %xmm3

    movdqa  %xmm3, %xmm2
    movdqa  %xmm3, %xmm4
    movdqa  %xmm3, %xmm5
    psrld   $1, %xmm2   # >>1
    psrld   $2, %xmm4   # >>2
    psrld   $7, %xmm5   # >>7
    pxor    %xmm4, %xmm2
    pxor    %xmm5, %xmm2
    pxor    %xmm8, %xmm2
    pxor    %xmm2, %xmm3
    pxor    %xmm3, %xmm6  # result of first multiply in XMM6

    # Now XMM6 = U = C·H mod P. Compute V = (U ⊕ [0||len]) · H.
    # Prepare the length block L = [0 (64 bits) || 128 (64 bits)] since C length=128 bits.
    movq    $0, -224(%rbp)      # L[0..7] = 0
    movl    $0, -216(%rbp)      # L[8..11] = 0
    movl    $128, -212(%rbp)    # L[12..15] = 0x00000080 (128 bits)
    movdqu  -224(%rbp), %xmm8   # load length block
    movdqa  %xmm6, %xmm0        # XMM0 = U
    pxor    %xmm8, %xmm0        # XMM0 = U ⊕ L

    # Second GF multiply: XMM0 = U⊕L, XMM1 = H.
    movdqa  %xmm2, %xmm1        # reload H into XMM1
    movdqa  %xmm0, %xmm3
    pclmulqdq $0x00, %xmm1, %xmm3
    movdqa  %xmm0, %xmm4
    pclmulqdq $0x10, %xmm1, %xmm4
    movdqa  %xmm0, %xmm5
    pclmulqdq $0x01, %xmm1, %xmm5
    movdqa  %xmm0, %xmm6
    pclmulqdq $0x11, %xmm1, %xmm6
    pxor    %xmm5, %xmm4
    movdqa  %xmm4, %xmm5
    psrldq  $8, %xmm4
    pslldq  $8, %xmm5
    pxor    %xmm5, %xmm3
    pxor    %xmm4, %xmm6

    movdqa  %xmm3, %xmm7
    movdqa  %xmm6, %xmm8
    pslld   $1, %xmm3
    pslld   $1, %xmm6
    psrld   $31, %xmm7
    psrld   $31, %xmm8
    movdqa  %xmm7, %xmm9
    pslldq  $4, %xmm8
    pslldq  $4, %xmm7
    psrldq  $12, %xmm9
    por     %xmm7, %xmm3
    por     %xmm8, %xmm6
    por     %xmm9, %xmm6

    movdqa  %xmm3, %xmm7
    movdqa  %xmm3, %xmm8
    movdqa  %xmm3, %xmm9
    pslld   $31, %xmm7
    pslld   $30, %xmm8
    pslld   $25, %xmm9
    pxor    %xmm8, %xmm7
    pxor    %xmm9, %xmm7
    movdqa  %xmm7, %xmm8
    pslldq  $12, %xmm7
    psrldq  $4, %xmm8
    pxor    %xmm7, %xmm3

    movdqa  %xmm3, %xmm2
    movdqa  %xmm3, %xmm4
    movdqa  %xmm3, %xmm5
    psrld   $1, %xmm2
    psrld   $2, %xmm4
    psrld   $7, %xmm5
    pxor    %xmm4, %xmm2
    pxor    %xmm5, %xmm2
    pxor    %xmm8, %xmm2
    pxor    %xmm2, %xmm3
    pxor    %xmm3, %xmm6        # final GHASH = (U⊕L)·H in XMM6

    # Tag = E(K,J0) XOR GHASH. (E(K,J0) was saved in XMM9)
    movdqa  %xmm9, %xmm0
    pxor    %xmm6, %xmm0
    movdqu  %xmm0, (%r8)         # store 16-byte tag at R8

    # Epilogue: restore stack frame and return
    RET
SYM_FUNC_END(aes_gcm_encrypt)

