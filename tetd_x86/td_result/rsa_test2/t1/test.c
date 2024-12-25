#include <time.h>
#include "bignum.h"
#include "rsa.h"

int main(void){
/*
    srand(time(NULL));
    testStr2bignum();
    testAddition();
    testSubtraction();
    testMultiplication();
    testRemainder();
    testAdditionModulus();
    testMultiplicationModulus();
    testExponentialModulus();
    testFermat();
    testGenerateRandom();
    testGenerateRandomPrime();
*/
    testRSA(384);
    return 0;
}
