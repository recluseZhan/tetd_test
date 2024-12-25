#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/random.h>
#include <linux/slab.h>
#define ACCURACY 5
#define SINGLE_MAX 10000
#define EXPONENT_MAX 1000
#define BUF_SIZE 1024

/**
 * Computes a^b mod c
 */
int modpow(long long a, long long b, int c) {
    int res = 1;
    while(b > 0) {
        /* Need long multiplication else this will overflow...
        if(b & 1) {
            res = (res * a) % c;
        }
        b = b >> 1;
        a = (a * a) % c; /* Same deal here */
    }
    return res;
}

/**
 * Computes the Jacobi symbol, (a, n).
 */
int jacobi(int a, int n) {
    int twos, temp;
    int mult = 1;
    while(a > 1 && a != n) {
        a = a % n;
        if(a <= 1 || a == n) break;
        twos = 0;
        while(a % 2 == 0 && ++twos) a /= 2; /* Factor out multiples of 2 */
        if(twos > 0 && twos % 2 == 1) mult *= (n % 8 == 1 || n % 8 == 7) * 2 - 1;
        if(a <= 1 || a == n) break;
        if(n % 4 != 1 && a % 4 != 1) mult *= -1; /* Coefficient for flipping */
        temp = a;
        a = n;
        n = temp;
    }
    if(a == 0) return 0;
    else if(a == 1) return mult;
    else return 0; /* a == n => gcd(a, n) != 1 */
}

/**
 * Check whether a is a Euler witness for n */
int solovayPrime(int a, int n) {
    int x = jacobi(a, n);
    if(x == -1) x = n - 1;
    return x != 0 && modpow(a, (n - 1)/2, n) == x;
}

/**
 * Test if n is probably prime, using accuracy of k (k solovay tests). */
int probablePrime(int n, int k) {
    if (n == 2)
        return 1;
    else if (n % 2 == 0 || n == 1)
        return 0;
    while (k-- > 0) {
        int random_num;
        get_random_bytes(&random_num, sizeof(random_num));
        if (!solovayPrime(random_num % (n - 2) + 2, n))
            return 0;
    }
    return 1;
}

/**
 * Find a random (probable) prime between 3 and n - 1 */
int randPrime(int n) {
    int prime;
    get_random_bytes(&prime, sizeof(prime));
    prime = prime % n;
    n += n % 2; /* n needs to be even so modulo wrapping preserves oddness */
    prime += 1 - prime % 2;
    while(1) {
        if(probablePrime(prime, ACCURACY)) return prime;
        prime = (prime + 2) % n;
    }
}

/**
 * Compute gcd(a, b). */
int gcd(int a, int b) {
    int temp;
    while(b != 0) {
        temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Find a random exponent x between 3 and n - 1 such that gcd(x, phi) = 1.
 * this distribution is similarly nowhere near uniform.
 */
int randExponent(int phi, int n) {
    int e;
    get_random_bytes(&e, sizeof(e));
    e = e % n;
    while(1) {
        if(gcd(e, phi) == 1) return e;
        e = (e + 1) % n;
        if(e <= 2) e = 3;
    }
}

/**
 * Compute n^-1 mod m by extended euclidian method.
 */
int inverse(int n, int modulus) {
    int a = n, b = modulus;
    int x = 0, y = 1, x0 = 1, y0 = 0, q, temp;
    while(b != 0) {
        q = a / b;
        temp = a % b;
        a = b;
        b = temp;
        temp = x; x = x0 - q * x; x0 = temp;
        temp = y; y = y0 - q * y; y0 = temp;
    }
    if(x0 < 0) x0 += modulus;
    return x0;
}


int encode(int m, int e, int n) {
    return modpow(m, e, n);
}

/**
 * Decode cryptogram c using private exponent and public modulus, m = c^d mod n. */
int decode(int c, int d, int n) {
    return modpow(c, d, n);
}

/**
 * Encode the message of given length, using the public key (exponent, modulus)
 * The resulting array will be of size len/bytes, each index being the encryption
 * of "bytes" consecutive characters, given by m = (m1 + m2*128 + m3*128^2 + ..),
 * encoded = m^exponent mod modulus
 */
int* encodeMessage(int len, int bytes, char* message, int exponent, int modulus) {
    int *encoded = (int *)kmalloc((len / bytes) * sizeof(int), GFP_KERNEL);
    int x, i, j;
    for(i = 0; i < len; i += bytes) {
        x = 0;
        for(j = 0; j < bytes; j++) x += message[i + j] * (1 << (7 * j));
        encoded[i/bytes] = encode(x, exponent, modulus);
#ifndef MEASURE
        printk("%d ", encoded[i/bytes]);
#endif
    }
    return encoded;
}

/**
 * Decode the cryptogram of given length, using the private key (exponent, modulus)
 * Each encrypted packet should represent "bytes" characters as per encodeMessage.
 * The returned message will be of size len * bytes.
 */
int* decodeMessage(int len, int bytes, int* cryptogram, int exponent, int modulus) {
    int *decoded = (int *)kmalloc((len / bytes) * sizeof(int), GFP_KERNEL);
    int x, i, j;
    for(i = 0; i < len; i++) {
        x = decode(cryptogram[i], exponent, modulus);
        for(j = 0; j < bytes; j++) {
            decoded[i*bytes + j] = (x >> (7 * j)) % 128;
#ifndef MEASURE
            if(decoded[i*bytes + j] != '\0') 
                printk("%x ", decoded[i*bytes + j]);
#endif
        }
    }
    return decoded;
}
const uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
static int __init rsa_sign_init(void)
{   
    int len = 16;
    int p, q, n, phi, e, d, bytes;
    int *encoded, *decoded;
    while(1) {
        p = randPrime(SINGLE_MAX);
        q = randPrime(SINGLE_MAX);
        n = p * q;
        if(n < 128) {
            printk("Modulus is less than 128, cannot encode single bytes.");
        }
        else break;
    }
    if(n >> 21) bytes = 3;
    else if(n >> 14) bytes = 2;
    else bytes = 1;
    phi = (p - 1) * (q - 1);
    e = randExponent(phi, EXPONENT_MAX);
    d = inverse(e, phi);
    encoded = encodeMessage(len,bytes,key,e,n);
    decoded = decodeMessage(len/bytes, bytes, encoded, d, n);
    for(int i = 0; i< 16;i++)
        printk("%x ",decoded[i]);
    return 0;
}

static void __exit rsa_sign_exit(void)
{
    
}

module_init(rsa_sign_init);
module_exit(rsa_sign_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("RSA Signature Module");

