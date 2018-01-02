#include <iostream>
#include <cstddef>
#include <queue>
#include <string>

#include <cstdlib>
#include <ctime>

#include <pthread.h>

#include "worker_thread.hpp"
#include "command.hpp"
#include "matrix.hpp"
#include "zero_handle.hpp"
#include "display_cmd.hpp"
#include "arrow.hpp"

static ZeroHandle* zeroHandle = nullptr;

struct worker_thread_arg_t {
	pthread_mutex_t init_mutex;
	pthread_cond_t  init_cond;
	bool worker_done_init;
};

class SetStructureCommand : public Command<> {
	unsigned int k_m;
	FUMatrix mat;
	
	SetStructureCommand(unsigned int k, FUMatrix&& matrix) : k_m(k), mat(matrix) {}
	
public:
	static Command<>* create(unsigned int k, FUMatrix&& matrix) {return new SetStructureCommand(k, std::move(matrix));}
	
	virtual void run() {
		std::list<Arrow> voidArrows;
		std::list<Arrow> fullArrows;
		std::list<std::pair<unsigned int, unsigned int>> voidArrowsData;
		std::list<ArrowBox::ArrowInArrowBox*> voidArrowsPtr;
		std::list<std::pair<unsigned int, unsigned int>> fullArrowsData;
		std::list<ArrowBox::ArrowInArrowBox*> fullArrowsPtr;
		
		if (zeroHandle!=nullptr)
			postDeleteZeroHandle(*zeroHandle);
		
		try {
			lemma23(mat, k_m, voidArrows, fullArrows);
		} catch(std::string s) {
			std::cout << s << std::endl;
			return;
		}
		
		srand(12345678);
		unsigned int num_track = k_m;
		for (int k=0; k<100; k++) {
			unsigned int i, j;
			i = rand()%num_track;
			do {
				j = rand()%num_track;
			} while(i==j);
			fullArrows.push_back(Arrow(i, j));
		}
		
		num_track = mat.size()/2-k_m;
		for (int k=0; k<100; k++) {
			unsigned int i, j;
			i = rand()%num_track;
			do {
				j = rand()%num_track;
			} while(i==j);
			voidArrows.push_back(Arrow(i, j));
		}
		
		/*for (int i=0; i<10; i++) {
			fullArrows.push_back(Arrow(k_m/2, k_m/2-1));
			fullArrows.push_back(Arrow(k_m/2+1, k_m/2));
		}
		unsigned int l = k_m/2;
		bool even = false;
		while (l>0) {
			if (!even) {
				fullArrows.push_back(Arrow(k_m/2, k_m/2-l));
			} else {
				fullArrows.push_back(Arrow(k_m/2+l, k_m/2));
			}
			
			--l;
			even = !even;
		}*/
		
		/*voidArrows.push_back(Arrow(0, 1));
		voidArrows.push_back(Arrow(1, 0));
		voidArrows.push_back(Arrow(2, 3));
		voidArrows.push_back(Arrow(3, 2));
		
		fullArrows.push_back(Arrow(0, 3));
		fullArrows.push_back(Arrow(1, 2));
		fullArrows.push_back(Arrow(2, 1));
		fullArrows.push_back(Arrow(3, 0));*/
		
		
		zeroHandle = new ZeroHandle(k_m, mat, std::move(voidArrows), std::move(fullArrows), voidArrowsData, voidArrowsPtr, fullArrowsData, fullArrowsPtr);
		postDrawZeroHandle(zeroHandle->getPairing(), *zeroHandle, std::move(voidArrowsData), std::move(voidArrowsPtr), std::move(fullArrowsData), std::move(fullArrowsPtr));
	}
	
	virtual std::string name() const {return "SetStructureCommand";}
};

class OnPlayCommand : public Command<> {
public:
	static Command<>* create() {return new OnPlayCommand;}
	
	virtual void run() {
		if (zeroHandle!=nullptr) {
			zeroHandle->proposition28();
			/*srand(12345678);
			unsigned int tracks = zeroHandle->getVoidHandle().numberOfTracks();
			BiPermutation permutation(tracks);
			permutation.postAdd(std::make_pair(tracks-1, tracks-2));
			for (int k=0; k<100; k++) {
				permutation.postAdd(std::make_pair(static_cast<unsigned int>(rand())%(tracks-1), static_cast<unsigned int>(rand())%(tracks-1)));
			}
			zeroHandle->getVoidHandle().permute(permutation, true);
			tracks = zeroHandle->getFullHandle().numberOfTracks();
			permutation = BiPermutation(tracks);
			permutation.postAdd(std::make_pair(tracks-1, tracks-2));
			for (int k=0; k<100; k++) {
				permutation.postAdd(std::make_pair(static_cast<unsigned int>(rand())%(tracks-1), static_cast<unsigned int>(rand())%(tracks-1)));
			}
			zeroHandle->getFullHandle().permute(permutation, true);
			tracks = zeroHandle->getFullHandle().numberOfTracks();
			permutation = BiPermutation(tracks);
			permutation.postAdd(std::make_pair(tracks-1, tracks-2));
			for (int k=0; k<100; k++) {
				permutation.postAdd(std::make_pair(static_cast<unsigned int>(rand())%(tracks-1), static_cast<unsigned int>(rand())%(tracks-1)));
			}
			zeroHandle->getFullHandle().permute(permutation, false);
			tracks = zeroHandle->getVoidHandle().numberOfTracks();
			permutation = BiPermutation(tracks);
			permutation.postAdd(std::make_pair(tracks-1, tracks-2));
			for (int k=0; k<100; k++) {
				permutation.postAdd(std::make_pair(static_cast<unsigned int>(rand())%(tracks-1), static_cast<unsigned int>(rand())%(tracks-1)));
			}
			zeroHandle->getVoidHandle().permute(permutation, false);
			
			zeroHandle->getVoidHandle().emptyArrowBoxThroughZeroHandle(*zeroHandle, true);
			zeroHandle->getFullHandle().emptyArrowBoxThroughZeroHandle(*zeroHandle, true);
			zeroHandle->getVoidHandle().emptyArrowBoxThroughZeroHandle(*zeroHandle, false);
			zeroHandle->getFullHandle().emptyArrowBoxThroughZeroHandle(*zeroHandle, false);*/
			
			/*ArrowBox::ArrowInArrowBox& arsInArwBox = zeroHandle->getVoidHandle().getFirstArrowBox().pushArrowRet(Arrow(0, 1));
			postPushArrowCommand(zeroHandle->getVoidHandle().getFirstArrowBox(), arsInArwBox, 0, 1);
			ArrowBox::ArrowInArrowBox& arsInArwBox1 = zeroHandle->getVoidHandle().getFirstArrowBox().pushArrowRet(Arrow(0, 1));
			postPushArrowCommand(zeroHandle->getVoidHandle().getFirstArrowBox(), arsInArwBox1, 0, 1);
			
			MoveArrowsAcrossZeroHandle* cmd = MoveArrowsAcrossZeroHandle::create(*zeroHandle);
			std::vector<MoveArrowsAcrossZeroHandle::IterationData> itData;
			ArrowBox& arrowBox = zeroHandle->getVoidHandle().getFirstArrowBox();
			for (int i=0; i<2; i++) {
				itData.emplace_back(0, 1, true, false, zeroHandle->getVoidHandle());
				itData.emplace_back(3, 2, true, false, zeroHandle->getVoidHandle());
				itData.emplace_back(0, 3, false, true, zeroHandle->getFullHandle());
				itData.emplace_back(1, 2, false, true, zeroHandle->getFullHandle());
			}
			itData.emplace_back(0, 1, true, false, zeroHandle->getVoidHandle());
			std::vector<MoveArrowsAcrossZeroHandle::IterationData> itData1;
			for (int i=0; i<1; i++) {
				itData1.emplace_back(0, 1, true, false, zeroHandle->getVoidHandle());
				itData1.emplace_back(3, 2, true, false, zeroHandle->getVoidHandle());
				itData1.emplace_back(0, 3, false, true, zeroHandle->getFullHandle());
				itData1.emplace_back(1, 2, false, true, zeroHandle->getFullHandle());
			}
			itData1.emplace_back(0, 1, true, false, zeroHandle->getVoidHandle());
			cmd->addArrow(arrowBox, makeArrowInArrowBoxIndexed(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(arrowBox)), std::move(itData), 0, false);
			cmd->addArrow(arrowBox, makeArrowInArrowBoxIndexed(++ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(arrowBox)), std::move(itData1), 4, false);
			postMoveArrowsAcrossZeroHandle(cmd);*/
			
			/*std::cout << "begin lemma 30" << std::endl;
			int startTime = clock();
			zeroHandle->getVoidHandle().lemma30();
			int endTime = clock();
			std::cout << "end lemma 30: " << (1.0*(endTime-startTime))/CLOCKS_PER_SEC << std::endl;*/
			
			/*zeroHandle->getVoidHandle().transferArrowsToFirstArrowBox();
			
			ArrowBox& arrowBoxVoid = zeroHandle->getVoidHandle().getFirstArrowBox();
			ArrowBox::ArrowInArrowBoxIndexedIterator crossPos(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(arrowBoxVoid));
			++crossPos;
			++crossPos;
			++crossPos;
			arrowBoxVoid.removeArrowGenCrossing(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(arrowBoxVoid),
					crossPos, zeroHandle->getVoidHandle().getPermutation());
			
			zeroHandle->getVoidHandle().transferArrowToSecondArrowBox(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(arrowBoxVoid));
			zeroHandle->getVoidHandle().transferArrowToSecondArrowBoxResolveCrossing(--ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(arrowBoxVoid));*/
			
			/*ArrowBox& arrowBox = zeroHandle->getFullHandle().getFirstArrowBox();
			ArrowBox::ArrowInArrowBox& arsInArwBox = arrowBox.pushArrowRet(Arrow(23, 1));
			postPushArrowCommand(arrowBox, arsInArwBox, 23, 1);
			ArrowBox::ArrowInArrowBoxIndexedIterator it(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(arrowBox));
			std::cout << "begin lemma 29" << std::endl;
			arrowBox.lemma29(it);
			std::cout << "end lemma 29" << std::endl;*/
			
			/*auto arrow  = arrowBox.get(0);
			auto arrow1 = arrowBox.get(2);
			auto arrow2 = arrowBox.get(3);
			arrow2 = arrowBox.addArrow(arrow2, Arrow(0, 1));
			arrowBox.moveArrow(arrow2, arrow);
			postGenArrowAfterMoveCrossing(arrowBox, makeArrowInArrowBoxIndexed(**arrow, 0), makeArrowInArrowBoxIndexed(**arrow1, 2), **arrow2, 0, 1, true);
			
			arrow1 = arrowBox.get(0);
			arrow2 = arrowBox.addArrow(arrow1, Arrow(1, 2));
			arrowBox.moveArrow(arrow2, arrow);
			postGenArrowAfterMoveCrossing(arrowBox, makeArrowInArrowBoxIndexed(**arrow, 2), makeArrowInArrowBoxIndexed(**arrow1, 0), **arrow2, 1, 2, false);
			
			arrow1 = arrowBox.get(1);
			arrow2 = arrowBox.get(2);
			arrowBox.moveArrow(arrow2, arrow);
			arrow2 = arrowBox.addArrow(arrow, Arrow(0, 2));
			postGenArrowAfterMoveCrossing(arrowBox, makeArrowInArrowBoxIndexed(**arrow, 0), makeArrowInArrowBoxIndexed(**arrow1, 1), **arrow2, 0, 2, false);
			
			arrow1 = arrowBox.get(1);
			arrow2 = arrowBox.addArrow(arrow1, Arrow(2, 0));
			arrowBox.moveArrow(arrow1, arrow);
			postGenArrowAfterMoveCrossing(arrowBox, makeArrowInArrowBoxIndexed(**arrow, 2), makeArrowInArrowBoxIndexed(**arrow1, 1), **arrow2, 2, 0, true);
			
			arrow1 = arrowBox.get(0);
			arrowBox.moveArrow(arrow1, arrow);
			postMoveArrowInArrowBox(arrowBox, makeArrowInArrowBoxIndexed(**arrow, 2), makeArrowInArrowBoxIndexed(**arrow1, 0));
			
			postMoveMergeArrowsCommand(arrowBox, makeArrowInArrowBoxIndexed(**arrow, 0), makeArrowInArrowBoxIndexed(**arrow1, 1));
			arrowBox.removeArrow(arrow);
			arrowBox.removeArrow(arrow1);
			
			arrow  = arrowBox.get(3);
			arrow1 = arrowBox.get(2);
			postMoveMergeArrowsCommand(arrowBox, makeArrowInArrowBoxIndexed(**arrow, 3), makeArrowInArrowBoxIndexed(**arrow1, 2));
			arrowBox.removeArrow(arrow);
			arrowBox.removeArrow(arrow1);*/
		}
	}
	
	virtual std::string name() const {return "OnPlayCommand";}
};

static pthread_t worker_thread;
static std::queue<Command<>*> workerQueue;
static pthread_cond_t         workerCond;
static pthread_mutex_t        workerMutex;

static int shouldStop = 0;
static bool initialized = false;

static void* worker_main(void* arg);

static void* worker_main(void* x) {
	worker_thread_arg_t* arg = static_cast<worker_thread_arg_t*>(x);
	
	pthread_mutex_init(&workerMutex, NULL);
	pthread_cond_init (&workerCond,  NULL);
	
	pthread_mutex_lock(&(arg->init_mutex));
		arg->worker_done_init = true;
		pthread_cond_signal(&(arg->init_cond));
	pthread_mutex_unlock(&(arg->init_mutex));
	
	while (!shouldStop) {
		Command<>* cmd;
		
		pthread_mutex_lock(&workerMutex);
			while (workerQueue.empty() && !shouldStop)
				pthread_cond_wait(&workerCond, &workerMutex);
			
			if (!shouldStop) {
				cmd = workerQueue.front();
				workerQueue.pop();
			}
		pthread_mutex_unlock(&workerMutex);
		
		if (!shouldStop) {
			cmd->run();
			cmd->clear();
		}
	}
	
	pthread_cond_destroy(&workerCond);
	pthread_mutex_destroy(&workerMutex);
	
	while(!workerQueue.empty()) {
		workerQueue.front()->clear();
		workerQueue.pop();
	}
	
	return NULL;
}

int shouldWorkerStop() {
	return shouldStop;
}

void initWorker() {
	worker_thread_arg_t worker_arg;
	worker_arg.worker_done_init = false;
	pthread_cond_init(&(worker_arg.init_cond), NULL);
	pthread_mutex_init(&(worker_arg.init_mutex), NULL);
	
	pthread_create(&worker_thread, NULL, worker_main, &worker_arg);
	
	pthread_mutex_lock(&(worker_arg.init_mutex));
		while (!worker_arg.worker_done_init) 
			pthread_cond_wait(&(worker_arg.init_cond), &(worker_arg.init_mutex));
	pthread_mutex_unlock(&(worker_arg.init_mutex));
	
	pthread_mutex_destroy(&(worker_arg.init_mutex));
	pthread_cond_destroy(&(worker_arg.init_cond));
	
	initialized = true;
}

void stopWorker() {
	if (initialized) {
		initialized = false;
		shouldStop = 1;
		pthread_mutex_lock(&workerMutex);
		pthread_cond_signal(&workerCond);
		pthread_mutex_unlock(&workerMutex);
		pthread_join(worker_thread, NULL);
		
		if (zeroHandle!=nullptr) {
			delete zeroHandle;
		}
	}
}

void postOnPlay() {
	pthread_mutex_lock(&workerMutex);
	workerQueue.push(OnPlayCommand::create());
	pthread_cond_signal(&workerCond);
	pthread_mutex_unlock(&workerMutex);
}

void postSetStructure(unsigned int k, FUMatrix&& matrix) {
	pthread_mutex_lock(&workerMutex);
	workerQueue.push(SetStructureCommand::create(k, std::move(matrix)));
	pthread_cond_signal(&workerCond);
	pthread_mutex_unlock(&workerMutex);
}
