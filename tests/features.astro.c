#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int64_t abs(int64_t x);
int64_t classify(int64_t n);
int main();

int64_t abs(int64_t x) {
return ((x < 0LL) ? ({(0LL - x);}) : ({x;}));
}

int64_t classify(int64_t n) {
return (n == 0LL ? 0LL : (n == 1LL ? 1LL : 2LL));
}

int main() {
    const int64_t a = abs((0LL - 42LL));
    printf("%lld\n", a);
    const int64_t b = classify(1LL);
    printf("%lld\n", b);
    const int64_t c = (3LL + (4LL * 2LL));
    printf("%lld\n", c);
    const bool flag = true;
    printf("%s\n", (flag) ? "true" : "false");
    return 0LL;
}

