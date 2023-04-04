#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// 
int* allocate_random_array(int parallel, size_t n, double min, double max) {
    int* ptr = (int*)malloc(sizeof(int) * n);
    int *pp;
    double t1 = omp_get_wtime();
    unsigned short seed[3];

    if (parallel) {
        #pragma omp parallel shared(ptr), private(seed, pp)
        {
            pp = ptr;
            #pragma omp for schedule(runtime)
            for (size_t i=0; i < n; i++) {
                pp[i] =  min + erand48(seed);
            }
        }

    } else {
        for (size_t i=0; i < n; i++) {
            ptr[i] = min + erand48(seed) ;
        }
    }

    double t2 = omp_get_wtime();
    printf("%lf\n", (t2 - t1));
    return ptr;
}


int main(int argc, char **argv) {
    int parallel = atoi(argv[1]);
    size_t arr_size = strtoull(argv[2], NULL, 10);
    int* ptr = allocate_random_array(parallel, arr_size, 0, 10);
    free(ptr);
    return 0;
}




