#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <vector>
#include <mutex>
#include <algorithm>


typedef std::vector<double> bucket_v;

struct bucket_t {
    double l;
    double r;
    bucket_v container;
    std::mutex bucketMutex;
    int previousElements;
};

int get_bucket_id(double v, double min, double one_b_width) {
    return (int)((v - min) / one_b_width);
}

void sequence_sort(double *ptr, size_t n, double min, double max, bucket_t* buckets, size_t buckets_no) {
    double one_b_width = (max - min) / buckets_no;
    double border = min;
    for (int i=0; i<buckets_no; i++) {
        buckets[i].l = border;
        border += one_b_width;
        buckets[i].r = border;
    }

    double v;
    int b_id;
    for (int i=0; i<n; i++) {
        v = ptr[i];
        b_id = get_bucket_id(v, min, one_b_width);
        buckets[b_id].container.push_back(v);
    }

    for (int i=0; i<buckets_no; i++) {
        auto& bucket = buckets[i].container;
        sort(bucket.begin(), bucket.end());
    }

    int ptr_i = 0;
    for (int bucket_id=0; bucket_id < buckets_no; bucket_id++) {
        for (int i = 0; i < buckets[bucket_id].container.size(); i++) {
            ptr[ptr_i] = buckets[bucket_id].container[i];
            ptr_i++;
        }
    }
}

void alg2_parallel_sort(double *ptr, size_t n, double min, double max, bucket_t* buckets, size_t buckets_no) {
    double one_b_width = (max - min) / buckets_no;
    double border = min;
    for (int i=0; i<buckets_no; i++) {
        buckets[i].l = border;
        border += one_b_width;
        buckets[i].r = border;
    }

    double v;
    int b_id;
    #pragma omp for schedule(static)
    for (int i=0; i<n; i++) {
        v = ptr[i];
        b_id = get_bucket_id(v, min, one_b_width);
        buckets[b_id].bucketMutex.lock();
        buckets[b_id].container.push_back(v);
        buckets[b_id].bucketMutex.unlock();
    }

    #pragma omp barrier
    #pragma omp for schedule(static)
    for (int i=0; i<buckets_no; i++) {
        auto& bucket = buckets[i].container;
        sort(bucket.begin(), bucket.end());
    }

    #pragma omp barrier
    for (int i = 0; i < buckets_no; i++) {
        if (i == 0)
            buckets[i].previousElements = 0;
        else
            buckets[i].previousElements = buckets[i - 1].previousElements + buckets[i - 1].container.size();
    }

    #pragma omp for schedule(static)
    for (int bucket_id=0; bucket_id < buckets_no; bucket_id++) {
        int offset = buckets[bucket_id].previousElements;
        for (int i = 0; i < buckets[bucket_id].container.size(); i++) {
            ptr[offset + i] = buckets[bucket_id].container[i];
        }
    }
}

void printBuckets(bucket_t* buckets, size_t buckets_no) {
    for (int i=0; i<buckets_no; i++) {
        printf("%lf %lf :: ", buckets[i].l, buckets[i].r);
        for (int j=0; j < buckets[i].container.size(); j++) {
            printf("%lf ", buckets[i].container[j]);
        } 
        printf("\n");
    }
}

void printArray(double* arr, size_t n) {
    printf("%f", arr[0]);
    for (int i = 0; i < n; i++) {
        printf(", %f", arr[i]);
    }
    printf("\n");
}

bool is_bucket_sorted(double* arr, size_t n) {
    for (int i = 1; i < n; i++) {
        if (arr[i-1] > arr[i]) {
            return false;
        }
    }
    return true;
}

double* random_bucket_sort(int parallel, size_t n, double min, double max, size_t buckets_no) {
    double *ptr = (double*)malloc(sizeof(double) * n);
    double *pp;
    double t1 = omp_get_wtime();
    unsigned short seed[3];
    bucket_t* buckets = new bucket_t[buckets_no];
    if (parallel) {
        #pragma omp parallel shared(ptr, buckets), private(seed, pp)
        {
            pp = ptr;
            int tid = omp_get_thread_num();
            seed[0] = ((tid * tid + 15) * 3)/7;

            #pragma omp for schedule(static)
            for (size_t i=0; i < n; i++) {
                pp[i] =  min + erand48(seed) * (max- min);
            }
            alg2_parallel_sort(ptr, n, min, max, buckets, buckets_no);
        }
        /*printBuckets(buckets, buckets_no);*/
        /*printArray(ptr, n);*/

    } else {
        for (size_t i=0; i < n; i++) {
            ptr[i] = min + erand48(seed) * (max- min) ;
        }

        sequence_sort(ptr, n, min, max, buckets, buckets_no);
        /*printArray(ptr, n);*/
        /*printBuckets(buckets);*/
    }

    double t2 = omp_get_wtime();
    printf("%lf\n", (t2 - t1));
    if (is_bucket_sorted(ptr, n)) {
        printf("Bucket is sorted\n");
    } else {
        printf("Bucket is NOT sorted\n");
    }
    return ptr;
}


int main(int argc, char **argv) {
    int parallel = atoi(argv[1]);
    size_t arr_size = strtoull(argv[2], NULL, 10);
    double* ptr = random_bucket_sort(parallel, arr_size, 0, 10, 1000);
    free(ptr);
    return 0;
}