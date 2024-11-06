#include "rsa.h"
//bignum PKCS1v15_padding(bignum m, int n_len);
/*
 * Returns the keys generated for RSA encryption
 * */
void keygen(bignum * n, bignum * e, bignum * d, int len){
    bignum p, q, phi_n;
    bignum t0, t1, bgcd, tmp;
    bignum ONE = digit2bignum(1);

    p = genrandomprime(len);
    q = genrandomprime(len);

    while (compare(p, q) == 0) {
        free(q.tab);
        q = genrandomprime(len);
    }
    *n = mult(p, q);
    t0 = sub(p, ONE);
    t1 = sub(q, ONE);
    phi_n = mult(t0, t1);
    free(t0.tab);
    free(t1.tab);
    
    *e = digit2bignum(3);

    while (1) {
        bgcd = gcd(*e, phi_n);
        if (compare(bgcd, ONE) == 0) {
            free(bgcd.tab);

            *d = inverse(*e, phi_n);
            break;
        }

        int e_len;
        do {
            e_len = rand() % (length(*n));
        } while (e_len <= 1);

        do {
            free(e->tab);
            *e = genrandom(e_len);
        } while (iszero(*e) || isone(*e));
    }

    free(ONE.tab);
    free(p.tab);
    free(q.tab);
    free(phi_n.tab);
}
/*
 * Encrypts the input message m with public key e and public modulus n
 * */
bignum RSAencrypt(bignum m, bignum e, bignum n){
    return expmod(m, e, n);
}

/*
 * Decrypts the cipher c with private key d and public modulus n
 * */
bignum RSAdecrypt(bignum c, bignum d, bignum n){
    return expmod(c, d, n);
}

/*---------------------*/
/* PKCS#1 v1.5 Padding */
/*bignum PKCS1v15_padding(bignum m, int n_len) {
    int m_len = length(m);  // 获取消息的长度
    int padding_len = n_len - m_len - 3;  // 留出3字节用于填充前缀（0x00 0x02 0x00）

    if (padding_len < 8) {
        printf("Message too long for RSA encryption with the current modulus size!\n");
        exit(1);
    }

    unsigned char *pad = malloc(padding_len + 3);  // 填充字节数组
    pad[0] = 0x00;  // 消息前缀
    pad[1] = 0x02;  // 0x02表示填充数据部分

    // 随机填充，确保填充字节中没有零
    for (int i = 2; i < padding_len + 2; i++) {
        pad[i] = rand() % 255 + 1;  // 填充非零字节
    }
    
    pad[padding_len + 2] = 0x00;  // 消息分隔符

    // 创建一个新的bignum，用于存储填充后的消息
    bignum padded_m;
    padded_m.tab = malloc(n_len);
    memset(padded_m.tab, 0, n_len);  // 清空内存

    // 将消息和填充数据合并
    memcpy(padded_m.tab + (n_len - m_len - padding_len - 3), m.tab, m_len);  // 合并消息
    memcpy(padded_m.tab + 2, pad, padding_len + 3);  // 合并填充数据

    free(pad);
    return padded_m;
}
*/
/*---------------------*/
/*
 * Test method to check the proper functionality of RSA encryption
 * */
void testRSA(int len){
    //printf("\n*************************** Test RSA - Start ***************************\n");

    bignum n, e, d, m, c, decypted_m;

    m = genrandom(32);
   // m = genrandom(len - 5);
//    m = str2bignum("12345678901234");

    //printf("Generating key for RSA...\n");
    keygen(&n, &e, &d, len);
    //printf("The generated keys are...\n");
    //printf ("n = ");
    //printbignum(n);
    //printf ("e = ");
    //printbignum(e);
    //printf ("d = ");
    //printbignum(d);

    printf ("\n\nMessage is:");
    printbignum(m);

    //printf("\n(Sign) Encrypting the message...\n");
    c = RSAencrypt(m, e, n);
    printf ("Cipher is:");
    printbignum(c);

    //printf("\n\n(Verify) Decrypting the cipher...\n");
    decypted_m = RSAdecrypt(c, d, n);
    printf ("Decrypted Message is:");
    printbignum(decypted_m);
    printf("\n\n%s\n", (compare(m, decypted_m) == 0) ? "RSA encryption-decryption correct" : "RSA encryption-decryption wrong");

    free(n.tab);
    free(e.tab);
    free(d.tab);
    free(m.tab);
    free(c.tab);
    free(decypted_m.tab);
   // printf("\n*************************** Test RSA - End ***************************\n");

}
