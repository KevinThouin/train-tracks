#ifndef __WORKER_THREAD_HPP__
#define __WORKER_THREAD_HPP__

#include <pthread.h>

#ifdef __cplusplus
#include "matrix.hpp"
#endif

int shouldWorkerStop();
void initWorker();
void stopWorker();

void postOnPlay();

#ifdef __cplusplus
void postSetStructure(unsigned int k, FUMatrix&& matrix);
#endif

#endif /* __WORKER_THREAD_HPP__ */
