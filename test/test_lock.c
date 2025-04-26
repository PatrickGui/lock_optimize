#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// 定义互斥锁
pthread_mutex_t mutex;
// 共享计数器
int counter = 0;
// 每个线程执行的递增次数
#define ITERATIONS 10
#define NUM_THREADS 4

// 线程函数，用于递增计数器
void* increment_counter(void* arg) {
    int id = arg;
    for (int i = 0; i < ITERATIONS; i++) {
        // 加锁，确保同一时刻只有一个线程可以访问计数器
        pthread_mutex_lock(&mutex);
        counter++;
        usleep(100);
        printf("id: %d, counter value: %d\n", id, counter);
        // 解锁，允许其他线程访问计数器
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    // 初始化互斥锁
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return 1;
    }

    // 定义线程 ID 数组
    pthread_t threads[NUM_THREADS];

    // 创建两个线程
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, increment_counter, i) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    // 等待两个线程执行完毕
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Thread join failed");
            return 1;
        }
    }

    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);

    // 输出最终的计数器值
    printf("Final counter value: %d\n", counter);

    return 0;
}
    