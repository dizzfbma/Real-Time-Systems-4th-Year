/* pmg.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int is_prime(long n) {
    if (n < 2) return 0;
    for (long i = 2; i <= (long)sqrt(n); i++) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

int main() {
    long limit = 100000;  
    long prime_count = 0;
    clock_t start_time = clock();

    for (long num = 2; num < limit; num++) {
        if (is_prime(num)) {
            prime_count++;
        }
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Found %ld primes below %ld in %f seconds.\n", prime_count, limit, time_spent);
    return 0;
}

