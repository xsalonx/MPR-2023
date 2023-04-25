#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <string>

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
    return (int)((v - min) / one_b_width);
}

void print_buckets(bucket_arr_t &buckets) {
    for (int i=0; i<buckets.size(); i++) {
        printf("%d; :: ",i);
        for (int j=0; j < buckets[i].container.size(); j++) {
            printf("%lf ", buckets[i].container[j]);
        } 
        printf("\n");
    }
}

void initializeBuckets(bucket_arr_t* buckets, size_t buckets_no, size_t arr_size) {
    for (int i=0; i<buckets_no; i++) {
        (*buckets)[i].container.resize(arr_size / buckets_no);
    }
}


double* random_bucket_sort_parallel(size_t n, double min, double max, size_t buckets_no) {
    double *ptr = (double*)malloc(sizeof(double) * n);
    double *pp;
    double t[1024];
    int time_ctr = 0;

    unsigned short seed[3];
    bucket_arr_t last_buckets(buckets_no);
    bucket_arr_t* th_buckets[1024];


    #pragma omp parallel shared(ptr, last_buckets, th_buckets, min, max, buckets_no, time_ctr), private(seed, pp)
    {   
        if (omp_get_thread_num() == 0) t[time_ctr++] = omp_get_wtime();

        // array filling wih random
        pp = ptr;
        int tid = omp_get_thread_num();
        seed[0] = ((tid * tid + 15) * 3)/7;

        #pragma omp for schedule(runtime)
        for (size_t i=0; i < n; i++) {
            pp[i] =  min + erand48(seed) * (max - min);
        }

        if (omp_get_thread_num() == 0) t[time_ctr++] = omp_get_wtime();

        // filling buckets
        bucket_arr_t* my_buckets = new bucket_arr_t(buckets_no);
        // initializeBuckets(my_buckets, buckets_no, n);
        th_buckets[tid] = my_buckets;
        int bid;
        double v;
        #pragma omp for schedule(runtime)
        for (int i=0; i<n; i++) {
            v = ptr[i];
            bid = get_bucket_id(v, min, max, buckets_no);
            (*my_buckets)[bid].container.push_back(v);
        }
        // if (omp_get_thread_num() == 0) t[time_ctr++] = omp_get_wtime();
        // squashing buckets
        int k = 0;
        #pragma omp for schedule(runtime)
        for (bid=0; bid < buckets_no; bid++) {
            for (int thi=0; thi < omp_get_num_threads(); thi++) {
                for (int i=0; i < (*th_buckets[thi])[bid].container.size(); i++) {
                    last_buckets[bid].container.push_back((*th_buckets[thi])[bid].container[i]);
                }
            } 
        }

        if (omp_get_thread_num() == 0) t[time_ctr++] = omp_get_wtime();

        // buckets sorting
        #pragma omp barierr
        #pragma omp for schedule(runtime)
        for (bid=0; bid<buckets_no; bid++) {
            std::sort(last_buckets[bid].container.begin(), last_buckets[bid].container.end());
        }

        if (omp_get_thread_num() == 0) t[time_ctr++] = omp_get_wtime();

        // copying buckets to final array
        #pragma omp for schedule(runtime)
        for (bid=0; bid < buckets_no; bid++) {
            // amortised caluclating of start of index in final array
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
            // copying
            for (int i=0; i < last_buckets[bid].container.size(); i++){
                ptr[i + last_buckets[bid].subctr] = last_buckets[bid].container[i];
            }
        }

        if (omp_get_thread_num() == 0) t[time_ctr++] = omp_get_wtime();
    }


    // results printing

    std::string t_names[32] = {""
        ,"random_generating"
        ,"buckets_ins_filling_(squash)"
        ,"buckets_sorting"
        ,"buckets_to_main_array"};

    printf("%d_%s:%lf", 0, t_names[1].c_str(), t[1] - t[0]);
    for (int i=2; i<time_ctr; i++) {
        printf(",%d_%s:%lf", i - 1, t_names[i].c_str(), t[i] - t[i-1]);
    }
    return ptr;
}



double* random_bucket_sort_seq(size_t n, double min, double max, size_t buckets_no) {
    double t[1024];
    int time_ctr = 0;
    t[time_ctr++] = omp_get_wtime();

    double *ptr = (double*)malloc(sizeof(double) * n);
    double *pp;
    unsigned short seed[3];

    for (size_t i=0; i < n; i++) {
        seed[0] = 5;//omp_get_wtime();
        ptr[i] = min + erand48(seed) * (max - min) ;
    }

    t[time_ctr++] = omp_get_wtime();

    // filling buckets
    bucket_arr_t buckets(buckets_no);
    double v;
    int b_id;
    for (int i=0; i<n; i++) {
        v = ptr[i];
        b_id = get_bucket_id(v, min, max, buckets_no);
        buckets[b_id].container.push_back(v);
    }

    t[time_ctr++] = omp_get_wtime();

    // buckets sorting
    for (int i=0; i<buckets_no; i++) {
        std::sort(buckets[i].container.begin(), buckets[i].container.end());
    }

    t[time_ctr++] = omp_get_wtime();

    // passing to main array
    int k = 0;
    for (int i=0; i<buckets.size(); i++) {
        for (int j=0; j < buckets[i].container.size(); j++) {
            ptr[k] = buckets[i].container[j];
            k++;
        } 
    }

    t[time_ctr++] = omp_get_wtime();


    // results printing
    std::string t_names[32] = {""
        ,"random_generating"
        ,"buckets_ins_filling_(squash)"
        ,"buckets_sorting"
        ,"buckets_to_main_array"};

    printf("%d_%s:%lf", 0, t_names[1].c_str(), t[1] - t[0]);
    for (int i=2; i<time_ctr; i++) {
        printf(",%d_%s:%lf", i - 1, t_names[i].c_str(), t[i] - t[i-1]);
    }
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
    printf(",total_time:%lf\n", (t2 - t1));
    return res;
}

int main(int argc, char **argv) {
    int parallel = atoi(argv[1]);
    size_t arr_size = strtoull(argv[2], NULL, 10);
    size_t buckets_no = strtoull(argv[3], NULL, 10);
    printf("arr_s:%d,buckets:%d,", arr_size, buckets_no);
    double* ptr = random_bucket_sort(parallel, arr_size, 0, 10, buckets_no);
    bool sorted = std::is_sorted(ptr, ptr + arr_size);
    free(ptr);

    if (!sorted) {
        std::cout << "Not sorted" << std::endl;
        exit(1);
    }
    return 0;
}

