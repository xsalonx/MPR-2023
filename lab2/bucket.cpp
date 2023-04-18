#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <mutex>

typedef std::vector<double> bucket_v;

struct bucket_t {
    double l;
    double r;
    bucket_v container;
    std::mutex mut;
    int subctr = 0;
};

typedef std::vector<bucket_t> bucket_arr_t;


int get_bucket_id(double v, double min, double max, int buckets_no) {
    double one_b_width = (max - min) / buckets_no;
    int bid = (int)((v - min) / one_b_width);
    // printf("%lf -> %d  --%d\n", v, bid, bid == (int)v);
    // if (bid != (int)v) {
    //     exit(1);
    // }
    return bid;
}

void print_buckets(bucket_arr_t &buckets) {
    for (int i=0; i<buckets.size(); i++) {
        // printf("%d' %lf %lf :: ",i, buckets[i].l, buckets[i].r);
        printf("%d; :: ",i);
        for (int j=0; j < buckets[i].container.size(); j++) {
            printf("%lf ", buckets[i].container[j]);
        } 
        printf("\n");
    }
}


double* random_bucket_sort_parallel(size_t n, double min, double max, size_t buckets_no) {
    double *ptr = (double*)malloc(sizeof(double) * n);
    double *pp;
    double t1 = omp_get_wtime();
    unsigned short seed[3];
    
    bucket_arr_t last_buckets(buckets_no);
    bucket_arr_t* th_buckets[1024];


    #pragma omp parallel shared(ptr, last_buckets, th_buckets, min, max, buckets_no), private(seed, pp)
    {   
        // array filling wih random
        pp = ptr;
        int tid = omp_get_thread_num();
        seed[0] = ((tid * tid + 15) * 3)/7;

        #pragma omp for schedule(runtime)
        for (size_t i=0; i < n; i++) {
            pp[i] =  min + erand48(seed) * (max - min);
        }


        bucket_arr_t* my_buckets = new bucket_arr_t(buckets_no);
        th_buckets[tid] = my_buckets;
        // filling buckets
        int bid;
        double v;
        #pragma omp for schedule(runtime)
        for (int i=0; i<n; i++) {
            v = ptr[i];
            bid = get_bucket_id(v, min, max, buckets_no);
            (*my_buckets)[bid].container.push_back(v);
        }

        // buckets sorting with for catenating - TODO
        // for (int i=0; i<buckets_no; i++) {
        //     std::sort((*my_buckets)[i].container.begin(), (*my_buckets)[i].container.end());
        // }

        // passing to main array
        int k = 0;
        #pragma omp for schedule(runtime)
        for (bid=0; bid < buckets_no; bid++) {
            for (int thi=0; thi < omp_get_num_threads(); thi++) {
                for (int i=0; i < (*th_buckets[thi])[bid].container.size(); i++) {
                    last_buckets[bid].container.push_back((*th_buckets[thi])[bid].container[i]);
                }
            } 
        }

        // buckets sorting - TODO        
        #pragma omp for schedule(runtime)
        for (bid=0; bid<buckets_no; bid++) {
            std::sort(last_buckets[bid].container.begin(), last_buckets[bid].container.end());
        }

        // buckets to final array
        #pragma omp for schedule(runtime)
        for (bid=0; bid<buckets_no; bid++) {
            last_buckets[bid].subctr = 0;
            if (bid > 0) {
                if (last_buckets[bid - 1].subctr > 0) {
                    last_buckets[bid].subctr = last_buckets[bid - 1].subctr + last_buckets[bid - 1].container.size();
                } else {
                    for (int p_bid = 0; p_bid < bid; p_bid++) {
                        last_buckets[bid].subctr += last_buckets[p_bid].container.size();
                    }
                }
            }
            
            for (int i=0; i < last_buckets[bid].container.size(); i++){
                ptr[i + last_buckets[bid].subctr] = last_buckets[bid].container[i];
            }
        }


    }

    // print_buckets(last_buckets);
    double t2 = omp_get_wtime();
    // printf("seq total time = %lf\n", (t2 - t1));
    return ptr;
}



void sequence_sort(double *ptr, size_t n, double min, double max, size_t buckets_no) {
    bucket_arr_t buckets(buckets_no);
    double v;
    int b_id;
    // filling buckets
    for (int i=0; i<n; i++) {
        v = ptr[i];
        b_id = get_bucket_id(v, min, max, buckets_no);
        buckets[b_id].container.push_back(v);
    }

    // buckets sorting
    for (int i=0; i<buckets_no; i++) {
        std::sort(buckets[i].container.begin(), buckets[i].container.end());
    }

    // passing to main array
    int k = 0;
    for (int i=0; i<buckets.size(); i++) {
        for (int j=0; j < buckets[i].container.size(); j++) {
            ptr[k] = buckets[i].container[j];
            k++;
        } 
    }
    
}

double* random_bucket_sort_seq(size_t n, double min, double max, size_t buckets_no) {
    double *ptr = (double*)malloc(sizeof(double) * n);
    double *pp;
    double t1 = omp_get_wtime();
    unsigned short seed[3];

    for (size_t i=0; i < n; i++) {
        seed[0] = 5;//omp_get_wtime();
        ptr[i] = min + erand48(seed) * (max - min) ;
    }

    sequence_sort(ptr, n, min, max, buckets_no);

    double t2 = omp_get_wtime();
    // printf("seq total time = %lf\n", (t2 - t1));
    return ptr;
}

double* random_bucket_sort(int parallel, size_t n, double min, double max, size_t buckets_no) {
    double* res = nullptr;
    double t1 = omp_get_wtime();
    if (parallel) {
        res = random_bucket_sort_parallel(n, min, max, buckets_no);
    } else {
        res = random_bucket_sort_seq(n, min, max, buckets_no);
    }
    double t2 = omp_get_wtime();
    printf("total time = %lf\n", (t2 - t1));

    return res;
}



void print_arr(double *ptr, size_t arr_size) {
    printf("\nresults: ");
    for (int i=0; i<arr_size; i++) {
        printf("%lf ", ptr[i]);
    }
}

int main(int argc, char **argv) {
    int parallel = atoi(argv[1]);
    size_t arr_size = strtoull(argv[2], NULL, 10);
    size_t buckets_no = strtoull(argv[3], NULL, 10);
    double* ptr = random_bucket_sort(parallel, arr_size, 0, 10, buckets_no);
    // print_arr(ptr, arr_size);
    free(ptr);
    printf("\n");
    return 0;
}







    // printf("\n\n");
    // for (int i=0; i<1024; i++) {
    //     bucket_arr_t* r = th_buckets[i];
    //     if (r == nullptr)
    //         break;
    //     printf("\nBuckets of thread %d:\n", i);
    //     print_buckets(*r);
    // }


  // bucket_arr_t bs[10];
    // bs[0] = bucket_arr_t(10);
    // std::cout << bs[0][0].container.size() << std::endl;
    // #pragma omp parallel
    // {
    //     std::cout << omp_get_num_threads() << std::endl;
    // }
    //         std::cout << omp_get_num_threads() << std::endl;