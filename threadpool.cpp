#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include <thread>
#include <vector>

pthread_mutex_t thread_locker;
pthread_cond_t queue_signal;

class Task{
public:
	int sleep_period = 1;

	Task(){
		sleep_period = 1;
	}

	Task(int _sleep_period){
		sleep_period = _sleep_period;
	}

	void setSleepPeriod(int _sleep_period){
		sleep_period = _sleep_period;
	}

	void execute(){
		sleep(this->sleep_period);
	}

	void printDetails(){
		std::cout<<"Task completed execution. Time Taken : "<<sleep_period<<" s.";
	}
};

class ThreadPool{
public:
	std::queue<Task> taskQueue;
	int taskCount = 0;
	int numThreads = 0;
	std::vector<std::thread> worker_threads;

	ThreadPool(){
		numThreads = 4;
	}

	ThreadPool(int _num_threads){
		numThreads = _num_threads;
	}

	void submitTask(Task task){
		pthread_mutex_lock(&thread_locker);
		taskQueue.push(task);
		taskCount = taskQueue.size();
		pthread_mutex_unlock(&thread_locker);
		pthread_cond_signal(&queue_signal);
	}

	void executeTask(Task *task){
		task->execute();
		task->printDetails();
	}

	void startThread(int id){
		while(true){
			Task task;
			pthread_mutex_lock(&thread_locker);
			while (taskCount == 0) {
				pthread_cond_wait(&queue_signal, &thread_locker);
			}

			task = taskQueue.front();
			taskQueue.pop();
			taskCount = taskQueue.size();
			pthread_mutex_unlock(&thread_locker);
			executeTask(&task);
			std::cout<<" Thread id : "<<id<<std::endl;
		}
	}

	void startPool(){
		for(int i = 0 ; i < numThreads ; i++){
			std::thread th = std::thread(&ThreadPool::startThread, this, i+1);
			worker_threads.push_back(move(th));
		}
	}
};


int main(int argc, char* argv[]){

	pthread_mutex_init(&thread_locker, NULL);
	pthread_cond_init(&queue_signal, NULL);

	ThreadPool* threadPool = new ThreadPool(std::thread::hardware_concurrency());

	for(int i = 0 ; i < 9 ; i++){
		Task *t = new Task(i+1);
		threadPool->submitTask(*t);
	}

	threadPool->startPool();

	// pthread_mutex_destroy(&thread_locker);
	// pthread_cond_destroy(&queue_signal);
	return 0;
}