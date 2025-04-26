#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 定义两个互斥锁
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

// 线程 1 函数
void* thread1_function(void* arg) {
    // 线程 1 先获取 mutex1
    pthread_mutex_lock(&mutex1);
    printf("Thread 1 acquired mutex1.\n");
    sleep(1);  // 模拟执行操作，确保线程 2 有机会获取 mutex2

    // 线程 1 尝试获取 mutex2
    printf("Thread 1 is trying to acquire mutex2.\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 1 acquired mutex2.\n");

    // 释放锁
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

// 线程 2 函数
void* thread2_function(void* arg) {
    // 线程 2 先获取 mutex2
    pthread_mutex_lock(&mutex2);
    printf("Thread 2 acquired mutex2.\n");
    sleep(1);  // 模拟执行操作，确保线程 1 有机会获取 mutex1

    // 线程 2 尝试获取 mutex1
    printf("Thread 2 is trying to acquire mutex1.\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 2 acquired mutex1.\n");

    // 释放锁
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // 初始化互斥锁
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    // 创建线程
    pthread_create(&thread1, NULL, thread1_function, NULL);
    pthread_create(&thread2, NULL, thread2_function, NULL);

    // 等待线程结束
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // 销毁互斥锁
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    return 0;
}    