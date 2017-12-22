#ifndef __WORKER_THREAD_H__
#define __WORKER_THREAD_H__

#include <pthread.h>

#ifdef __cplusplus
#include "matrix.hpp"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int shouldWorkerStop();
void initWorker();
void stopWorker();

void postOnPlay();

#ifdef __cplusplus
void postSetStructure(unsigned int k, FUMatrix&& matrix);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __WORKER_THREAD_H__ */
