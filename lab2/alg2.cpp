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

namespace parallel
{
void generateRandomValues(double * ptr, size_t n, double min, double max, unsigned short * seed) {
    double *pp = ptr;
    int tid = omp_get_thread_num();
    seed[0] = ((tid * tid + 15) * 3)/7;

    #pragma omp for schedule(guided, 5)
    for (size_t i=0; i < n; i++) {
        pp[i] =  min + erand48(seed) * (max- min);
    }
}

double getBucketWidth(double min, double max, size_t buckets_no) {
    return (max - min) / buckets_no;
}

void initializeBuckets(double min, double max, bucket_t* buckets, size_t buckets_no) {
    double one_b_width = getBucketWidth(min, max, buckets_no);
    double border = min;
    for (int i=0; i<buckets_no; i++) {
        buckets[i].l = border;
        border += one_b_width;
        buckets[i].r = border;
    }
}

void writeValuesToBuckets(double *ptr, size_t n, double min, double max, bucket_t* buckets, size_t buckets_no) {
    double one_b_width = getBucketWidth(min, max, buckets_no);
    double v;
    int b_id;
    #pragma omp for schedule(guided, 5)
    for (int i=0; i<n; i++) {
        v = ptr[i];
        b_id = get_bucket_id(v, min, one_b_width);
        buckets[b_id].bucketMutex.lock();
        buckets[b_id].container.push_back(v);
        buckets[b_id].bucketMutex.unlock();
    }
}

void sortBuckets(bucket_t* buckets, size_t buckets_no) {
    #pragma omp for schedule(guided, 5)
    for (int i=0; i<buckets_no; i++) {
        auto& bucket = buckets[i].container;
        sort(bucket.begin(), bucket.end());
    }
}

void mergeBuckets(double *ptr, bucket_t* buckets, size_t buckets_no) {
    for (int i = 0; i < buckets_no; i++) {
        if (i == 0)
            buckets[i].previousElements = 0;
        else
            buckets[i].previousElements = buckets[i - 1].previousElements + buckets[i - 1].container.size();
    }

    #pragma omp for schedule(guided, 5)
    for (int bucket_id=0; bucket_id < buckets_no; bucket_id++) {
        int offset = buckets[bucket_id].previousElements;
        for (int i = 0; i < buckets[bucket_id].container.size(); i++) {
            ptr[offset + i] = buckets[bucket_id].container[i];
        }
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

double average(double arr[], int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum / size;
}

double* random_bucket_sort(int parallel, size_t n, double min, double max, size_t buckets_no) {
    double *ptr = (double*)malloc(sizeof(double) * n);
    double totalTimerStart = omp_get_wtime(), totalTimerEnd;
    double timerStart = omp_get_wtime(), timerEnd;
    uint numOfThreads = omp_get_num_threads();
    double timers[6][numOfThreads];
    unsigned short seed[3];
    bucket_t* buckets = new bucket_t[buckets_no];
    if (parallel) {
        #pragma omp parallel shared(ptr, buckets), private(seed)
        {
            uint threadId = omp_get_thread_num();
            timers[0][threadId] = omp_get_wtime();

            parallel::generateRandomValues(ptr, n, min, max, seed);
            
            timers[1][threadId] = omp_get_wtime();

            parallel::initializeBuckets(min, max, buckets, buckets_no);
            
            timers[2][threadId] = omp_get_wtime();

            parallel::writeValuesToBuckets(ptr, n, min, max, buckets, buckets_no);
            
            timers[3][threadId] = omp_get_wtime();

            parallel::sortBuckets(buckets, buckets_no);

            timers[4][threadId] = omp_get_wtime();

            parallel::mergeBuckets(ptr, buckets, buckets_no);
            
            timers[5][threadId] = omp_get_wtime();
        }

        printf("Time taken by generating random values: %lf\n", (average(timers[1], numOfThreads) - average(timers[0], numOfThreads)));
        printf("Time taken by initializing buckets: %lf\n", (average(timers[2], numOfThreads) - average(timers[1], numOfThreads)));
        printf("Time taken by writing values to buckets: %lf\n", (average(timers[3], numOfThreads) - average(timers[2], numOfThreads)));
        printf("Time taken by sorting inside buckets: %lf\n", (average(timers[4], numOfThreads) - average(timers[3], numOfThreads)));
        printf("Time taken by merging buckets: %lf\n", (average(timers[5], numOfThreads) - average(timers[4], numOfThreads)));

    }

    totalTimerEnd = omp_get_wtime();
    printf("Total taken time: %lf\n", (totalTimerEnd - totalTimerStart));
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