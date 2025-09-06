//
// Created by Yue Xue  on 8/20/25.
//

#ifndef LOXMUTEX_H
#define LOXMUTEX_H
#include <pthread/pthread.h>
#include <sys/_pthread/_pthread_mutex_t.h>

class LoxMutex {
public:
    explicit LoxMutex() {
        pthread_mutex_init(&mutex_, nullptr);
    }
    ~LoxMutex() {
        pthread_mutex_destroy(&mutex_);
    }
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
    pthread_mutex_t &mutex() {
        return mutex_;
    }
private:
    pthread_mutex_t mutex_;
};

class LoxCondition {
public:
    explicit LoxCondition() {
        pthread_cond_init(&cond_, nullptr);
    }
    ~LoxCondition() {
        pthread_cond_destroy(&cond_);
    }
    void wait(LoxMutex &mutex) {
        pthread_cond_wait(&cond_, &mutex.mutex());
    }
    void notify() {
        pthread_cond_signal(&cond_);
    }
private:
    pthread_cond_t cond_;
};

#endif //LOXMUTEX_H
