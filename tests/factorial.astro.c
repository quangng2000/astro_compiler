#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int64_t factorial(int64_t n);
int main();

int64_t factorial(int64_t n) {
return (n == 0LL ? 1LL : (n * factorial((n - 1LL))));
}

int main() {
    const int64_t result = factorial(5LL);
    printf("%lld\n", result);
    return 0LL;
}

