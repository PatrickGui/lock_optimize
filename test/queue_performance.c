#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define NUM_THREADS 10
#define ITERATIONS 100000

// 传统锁队列
typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    pthread_mutex_t lock;
} LockedQueue;

void locked_queue_init(LockedQueue* q) {
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
}

void locked_queue_enqueue(LockedQueue* q, int value) {
    pthread_mutex_lock(&q->lock);
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = value;
    new_node->next = NULL;
    if (q->tail == NULL) {
        q->head = q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    pthread_mutex_unlock(&q->lock);
}

int locked_queue_dequeue(LockedQueue* q) {
    pthread_mutex_lock(&q->lock);
    if (q->head == NULL) {
        pthread_mutex_unlock(&q->lock);
        return -1;
    }
    Node* temp = q->head;
    int value = temp->data;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(temp);
    pthread_mutex_unlock(&q->lock);
    return value;
}

void locked_queue_destroy(LockedQueue* q) {
    while (locked_queue_dequeue(q) != -1);
    pthread_mutex_destroy(&q->lock);
}

// 无锁队列
typedef struct LFNode LFNode;

struct LFNode {
    int data;
    _Atomic(LFNode*) next;
};

typedef struct {
    _Atomic(struct LFNode *) head;
    _Atomic(struct LFNode *) tail;
} LockFreeQueue;

void lock_free_queue_init(LockFreeQueue* q) {
    LFNode* dummy = (LFNode*)malloc(sizeof(LFNode));
    atomic_store(&q->head, dummy);
    atomic_store(&q->tail, dummy);
    atomic_store(&dummy->next, NULL);
}

void lock_free_queue_enqueue(LockFreeQueue* q, int value) {
    LFNode* new_node = (LFNode*)malloc(sizeof(LFNode));
    new_node->data = value;
    atomic_store(&new_node->next, NULL);
    LFNode* old_tail;
    LFNode* expected;
    do {
        old_tail = atomic_load(&q->tail);
        expected = atomic_load(&old_tail->next);
    } while (!atomic_compare_exchange_weak(&old_tail->next, &expected, new_node));
    atomic_compare_exchange_strong(&q->tail, &old_tail, new_node);
}

int lock_free_queue_dequeue(LockFreeQueue* q) {
    LFNode* old_head;
    LFNode* next;
    do {
        old_head = atomic_load(&q->head);
        next = atomic_load(&old_head->next);
        if (next == NULL) {
            return -1;
        }
    } while (!atomic_compare_exchange_weak(&q->head, &old_head, next));
    int value = next->data;
    free(old_head);
    return value;
}

void lock_free_queue_destroy(LockFreeQueue* q) {
    while (lock_free_queue_dequeue(q) != -1);
    free(atomic_load(&q->head));
}

// 计算时间差的辅助函数
long long get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
}

// 传统锁队列线程函数
void* locked_queue_worker(void* arg) {
    LockedQueue* q = (LockedQueue*)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        locked_queue_enqueue(q, i);
        locked_queue_dequeue(q);
    }
    return NULL;
}

// 无锁队列线程函数
void* lock_free_queue_worker(void* arg) {
    LockFreeQueue* q = (LockFreeQueue*)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        lock_free_queue_enqueue(q, i);
        lock_free_queue_dequeue(q);
    }
    return NULL;
}

int main() {
#if __STDC_VERSION__ >= 201112L
    printf("C11 standard is enabled.\n");
#elif __STDC_VERSION__ >= 199901L
    printf("C99 standard is enabled.\n");
#elif __STDC_VERSION__ >= 199409L
    printf("C90 standard is enabled.\n");
#else
    printf("Pre-C90 standard is enabled.\n");
#endif
    pthread_t threads[NUM_THREADS];
    struct timespec start, end;

    // 测试传统锁队列
    LockedQueue locked_q;
    locked_queue_init(&locked_q);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, locked_queue_worker, &locked_q);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long locked_time = get_time_diff(start, end);
    locked_queue_destroy(&locked_q);

    printf("传统锁队列：耗时 %lld 纳秒。\n", locked_time);

    // 测试无锁队列
    LockFreeQueue lock_free_q;
    lock_free_queue_init(&lock_free_q);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, lock_free_queue_worker, &lock_free_q);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long lock_free_time = get_time_diff(start, end);
    lock_free_queue_destroy(&lock_free_q);

    printf("无锁队列：耗时 %lld 纳秒。\n", lock_free_time);

    return 0;
}