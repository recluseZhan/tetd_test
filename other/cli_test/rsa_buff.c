#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/gfp.h>
#include <linux/err.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <crypto/skcipher.h>
#include <crypto/akcipher.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/highmem.h>
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
const char *priv_key =
    "----BEGIN PRIVATE KEY-----"
"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"
"jZ5FRtkkt4DYQET268g1t7c="
"-----END PRIVATE KEY-----";

 
const char *pub_key =
    "-----BEGIN PUBLIC KEY-----"
"MIIBojANBgkqhkiG9w0BAQEFAAOCAY8AMIIBigKCAYEArwULivRxEMFAqtIRgZtN"
"GPiZpGr6MeViKE1z/f0fykDjwiViKYV2C9B7/aIjT565PdkUAp9wIgDSmYV637yd"
"VPR0KIQVQ5yFe3/eCoTwoIlZ+uXjyYDGolKxdNxQkNyMMEhXuJD9ic4Jri5q9BUJ"
"AFFbi3OMHi899q0+mgNJ/EEb+DI6A5X6Nu9UPtoDo7tPJJLu1QNwBnB31yVUNXdj"
"J8k39PDiwRlAbSq8AfetuAGthBvWJQwBVhP7svFA1bLJq3/jzPIfZ4HX+2JwR7ba"
"DrI51Ueq95YqCjoDJUTGNPZvuGGROf8kMeKgGAbcFnFXaIO2d1NtTL2c56AfQlf/"
"jG8ODE1DlgJhdOrCQqL1w9VxP1uW6EspLbYTQDsO/ZFGM1FqUzruhQxxpeq7tWlt"
"G3oTVkW+HpYPewTMPUCXsyFDAXKZfCnPFPrDovYOPiLDow4KewCobtFMeVWZJ36s"
"DpsqnnJ9WRfUJMPQQRcpb7ddw4RGUlWgk0EaFN752y0BAgMBAAE="
"-----END PUBLIC KEY-----";

 
char *crypted = NULL;
int crypted_len = 0;
 
struct tcrypt_result {
    struct completion completion;
    int err;
};
 
struct akcipher_testvec {
    unsigned char *key;
    unsigned char *msg;
    unsigned int key_size;
    unsigned int msg_size;
};
 
static inline  void hexdump(unsigned char *buf,unsigned int len) {
    while(len--)
        printk(KERN_CONT "%02x",*buf++);
    printk("\n");
}
 
static void tcrypt_complete(struct crypto_async_request *req, int err)
{
    struct tcrypt_result *res = req->data;
 
    if (err == -EINPROGRESS)
        return;
 
    res->err = err;
    complete(&res->completion);
}
 
static int wait_async_op(struct tcrypt_result *tr, int ret)
{
    if (ret == -EINPROGRESS || ret == -EBUSY) {
        wait_for_completion(&tr->completion);
        reinit_completion(&tr->completion);
        ret = tr->err;
    }
    return ret;
}
 
static int uf_akcrypto(struct crypto_akcipher *tfm,
                         void *data, int datalen, int phase)
{
    void *xbuf = NULL;
    struct akcipher_request *req;
    void *outbuf = NULL;
    struct tcrypt_result result;
    unsigned int out_len_max = 0;
    struct scatterlist src, dst;
    const int pub_key_len = strlen(pub_key);
    const int priv_key_len = strlen(priv_key);
    int err = -ENOMEM;
    xbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!xbuf)
         return err;
 
    req = akcipher_request_alloc(tfm, GFP_KERNEL);
    if (!req)
         goto free_xbuf;
 
    init_completion(&result.completion);
 
    if (!phase){  //test
         err = crypto_akcipher_set_pub_key(tfm, pub_key, pub_key_len);
    }else{
         err = crypto_akcipher_set_priv_key(tfm, priv_key, priv_key_len);
    }
//  err = crypto_akcipher_set_priv_key(tfm, priv_key, priv_key_len);
    if (err){
        printk("set key error! %d,,,,,%d\n", err,phase);
        goto free_req;
    }
 
    err = -ENOMEM;
    out_len_max = crypto_akcipher_maxsize(tfm);
    outbuf = kzalloc(out_len_max, GFP_KERNEL);
    if (!outbuf)
         goto free_req;
 
    if (WARN_ON(datalen > PAGE_SIZE))
         goto free_all;
 
    memcpy(xbuf, data, datalen);
    sg_init_one(&src, xbuf, datalen);
    sg_init_one(&dst, outbuf, out_len_max);
    akcipher_request_set_crypt(req, &src, &dst, datalen, out_len_max);
    akcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG,tcrypt_complete, &result);
 
    unsigned long t1,t2;
    int en_ret;
    if (phase){
        t1=urdtsc();
        en_ret=crypto_akcipher_encrypt(req);
        t2=urdtsc();
        printk("signature time(ns) : %ld \n ", (t2-t1)*5/17);
        err = wait_async_op(&result, en_ret);
        if (err) {
            pr_err("alg: akcipher: encrypt test failed. err %d\n", err);
            goto free_all;
        }
        memcpy(crypted,outbuf,out_len_max);
        crypted_len = out_len_max;
        hexdump(crypted, out_len_max);
    }else{
        err = wait_async_op(&result, crypto_akcipher_decrypt(req));
        if (err) {
            pr_err("alg: akcipher: decrypt test failed. err %d\n", err);
            goto free_all;
        }
        hexdump(outbuf, out_len_max);
    }
 
free_all:
    kfree(outbuf);
free_req:
    akcipher_request_free(req);
free_xbuf:
    kfree(xbuf);
    return err;
}
 
 
static int userfaultfd_akcrypto(void *data, int datalen, int phase)
{
     struct crypto_akcipher *tfm;
     int err = 0;
 
     tfm = crypto_alloc_akcipher("rsa", CRYPTO_ALG_INTERNAL, 0);
     if (IS_ERR(tfm)) {
             pr_err("alg: akcipher: Failed to load tfm for rsa: %ld\n", PTR_ERR(tfm));
             return PTR_ERR(tfm);
     }
     err = uf_akcrypto(tfm,data,datalen,phase);
 
     crypto_free_akcipher(tfm);
     return err;
}
#define DATA_SIZE 4096
static int __init test_init(void)
{  
    crypted = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!crypted){
        printk("crypted kmalloc error\n");
        return -1;
    }
    //const char *msg = "\x54\x85\x9b\x34\x2c\x49\xea\x2a\x54\x85\x9b\x34\x2c\x49\xea\x2a\x54\x85\x9b\x34\x2c\x49\xea\x2a";
    uint8_t *msg;
    msg = kmalloc(DATA_SIZE,GFP_KERNEL);
    get_random_bytes(msg,DATA_SIZE);
    const int msg_len = strlen(msg);
    userfaultfd_akcrypto(msg,msg_len,1);
    //userfaultfd_akcrypto(crypted,crypted_len,0);
    kfree(crypted);
    return 0;
}
 
static void __exit test_exit(void)
{
 
}
 
module_init(test_init);
module_exit(test_exit);
 
MODULE_LICENSE("GPL");
