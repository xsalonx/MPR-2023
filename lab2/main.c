#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// klauzula schedule, chunk 5 ustawień

int* allocate_random_array(int parallel, size_t n, double min, double max) {
    int* ptr = (int*)malloc(sizeof(int) * n);
    int *pp;
    double t1 = omp_get_wtime();
    unsigned short seed[3];

    if (parallel) {
        #pragma omp parallel shared(ptr), private(seed, pp)
        {
            pp = ptr;
            #pragma omp for nowait, schedule(runtime)
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
    printf("%f\n", (t2 - t1));
    return ptr;
}

// void hello_world() {
//     int var1, var2, var3;
//     printf("Threads no: %d\n", omp_get_num_threads());
//     printf("My number is %d\n\n", omp_get_thread_num());

//     #pragma omp parallel private (var1, var2) shared (var3)
//     {
//         printf("--Threads no: %d\n", omp_get_num_threads()) ;
//         printf("--My number is %d\n", omp_get_thread_num()) ;
//     }
//     printf("\n");
//     printf("Threads no: %d\n", omp_get_num_threads());
//     printf("My number is %d\n\n", omp_get_thread_num());
// }

int main(int argc, char **argv) {
    // hello_world();
    size_t arr_size = strtoull(argv[1], NULL, 10);
    int parallel;
    // if (argc > 2) {
        parallel = atoi(argv[2]);
    // } else {
    //     parallel = 0
    // }
    int* ptr = allocate_random_array(parallel, arr_size, 0, 10);
    free(ptr);
    return 0;
}




