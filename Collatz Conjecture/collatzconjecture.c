#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char* argv[]) {
    long long Nmax = atoll(argv[1]);
    long long Imax = Nmax;
    long long n;
    long long i, j;
    long long high = 0;
    double startTime, endTime;
    startTime = omp_get_wtime();
    #pragma omp parallel for schedule(static, 500) private (n, i) reduction(max:high)
    for (j = 1; j < Nmax; j++) {
        n=j;
        for (i = 1; i < Imax; i++) {
            if (n % 2 == 0) {
                n = n/2;
            } else {
                n = 3*n + 1;
            }
            if (n > high) high = n;
            if (n==1) break;
        }
    }
    endTime = omp_get_wtime();
    printf("High: %lld\n", high);
    printf("Runtime: %.16f\n", endTime - startTime);
    return 0;
}
