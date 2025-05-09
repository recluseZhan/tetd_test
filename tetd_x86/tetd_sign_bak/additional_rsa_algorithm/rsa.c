#include "rsa.h"
// Returns the keys generated for RSA encryption
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

// Encrypts the input message m with public key e and public modulus n.
bignum RSAencrypt(bignum m, bignum e, bignum n){
    return expmod(m, e, n);
}

// Decrypts the cipher c with private key d and public modulus n.
bignum RSAdecrypt(bignum c, bignum d, bignum n){
    return expmod(c, d, n);
}

void testRSA(int len){
    bignum n, e, d, m, c, decypted_m;
    m = genrandom(32);

    keygen(&n, &e, &d, len);

    printf ("\n\nMessage is:");
    printbignum(m);

    c = RSAencrypt(m, e, n);
    printf ("Cipher is:");
    printbignum(c);

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
}
