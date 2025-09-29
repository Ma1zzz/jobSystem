#define DEBUGMODE false
#include "jobs.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <limits.h>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct jobData {
  std::function<void()> function;
  std::function<void()> dependency;
  int priority;
};

static std::condition_variable threadCondition;
static std::condition_variable isWorkDone;

static bool shouldShutDown = false;

static std::mutex jobsQueueMutex;
static std::mutex mutex;

static std::queue<std::function<void()>> loopJobs;
static std::vector<jobData> getJobQueue;
/*static std::vector<jobData> &getJobQueue() {
  static std::vector<jobData> jobQueue;
  // static std::queue<jobData> jobQueue;
  return jobQueue;:w
}*/

static std::atomic<int> loopJobsCount = 0;
static std::atomic<int> loopJobsLeftToComplete = 0;

static std::vector<std::thread> threads;

static std::atomic<int> idleThreads;
// static int idleThreads;
static std::atomic<int> usableThreads;
// static int usableThreads;
//

std::counting_semaphore<std::numeric_limits<int>::max()> sem(0);

void reqJobs(std::function<void()> func, std::function<void()> dep, int pri) {
  jobsQueueMutex.lock();
  // std::unique_lock<std::mutex> lock(jobsQueueMutex);
  getJobQueue.push_back({func, dep, pri});
  // getJobQueue().push({func, dep, pri});
  // lock.unlock()

  sem.release();
  jobsQueueMutex.unlock();
  //   threadCondition.notify_one();
}

static jobData pickJob() {
  /*
    std::vector<jobData> jobVector;
    std::unique_lock<std::mutex> lock(jobsQueueMutex);

    jobData jobToDo;
    jobToDo = getJobQueue().front();

    std::cout << getJobQueue().size() << std::endl;
    for (int i = 0; i < getJobQueue().size(); i++) {

      if (getJobQueue()[i].priority < jobToDo.priority) {
        jobToDo = getJobQueue()[i];
      }
    }
    lock.unlock();

    return jobToDo;*/
}

static void createWorkerThread() {

  while (!shouldShutDown) {

    idleThreads++;
    jobsQueueMutex.lock();
    if (getJobQueue.empty()) {

      jobsQueueMutex.unlock();
      // std::cout << "empty" << std::endl;
      sem.acquire();
    } else {
      jobsQueueMutex.unlock();
    }
    if (shouldShutDown) {
      // std::cout <<"\n shutting down" << std::endl;
      break;
    }

    // std::cout << getJobQueue().size() << std::endl;

    idleThreads--;

    // std::unique_lock<std::mutex> lock(jobsQueueMutex);
    jobsQueueMutex.lock();
    if (!loopJobs.empty()) {
      auto loopJob = loopJobs.front();
      loopJobs.pop();
      jobsQueueMutex.unlock();
      loopJob();
      isWorkDone.notify_one();
      continue;
    }

    jobData jobdata = getJobQueue.front();
    // jobData jobdata = pickJob();
    auto job = jobdata.function;
    auto dep = jobdata.dependency;

    getJobQueue.erase(getJobQueue.begin() + 0);
    jobsQueueMutex.unlock();

    if (dep != nullptr) {
      dep();
    }
    job();
    // std::cout << "job is done " << std::endl;
    // isWorkDone.notify_one();
  }
}

void initJobsSystem() {
  // usableThreads = std::thread::hardware_concurrency();
  usableThreads = 6; // for testing

  if (usableThreads == 1) {
    throw std::runtime_error("Not enough threads for multithreading.");
  }
#if DEBUGMODE
  std::cout << " total threads: " << usableThreads << std::endl;
#endif

  usableThreads -= 1;

  if (usableThreads > 4)
    usableThreads -= 1;

#if DEBUGMODE
  std::cout << " Threads program will use : " << usableThreads << std::endl;
#endif

  threads.resize(usableThreads);
  // freeThreads.resize(usableThreads, true);
  shouldShutDown = false;
}

static void submitloopJob(std::function<void()> func) {
  std::unique_lock<std::mutex> lock(jobsQueueMutex);
  loopJobs.push(func);
  lock.unlock();
  threadCondition.notify_one();
}

static void waitForLoopJobs() {
  while (true) {
    jobsQueueMutex.lock();
    if (loopJobs.empty()) {
      jobsQueueMutex.unlock();
      break;
    }
    // std::cout << "working on loop jobs " <<  std::endl;
    // std::unique_lock<std::mutex> lock(jobsQueueMutex);
    auto job = loopJobs.front();
    loopJobs.pop();
    jobsQueueMutex.unlock();
    job();
  }
}

void parallelLoop(int stat, int end, std::function<void(int)> code,
                  int jobsToCreate, bool wait) {
  int count = end - stat;
  int remainder = count % jobsToCreate;
  count -= remainder;
  int loopChunk = (count / (jobsToCreate));
  loopJobsCount += jobsToCreate;
  // loopJobsLeftToComplete += jobsToCreate;

  for (int job = 0; job < jobsToCreate; job++) {
    int countStart = loopChunk * job + stat;
    int countEnd = loopChunk + countStart;

    if (job == jobsToCreate - 1)
      countEnd += remainder;

    submitloopJob([code, countStart, countEnd]() {
      for (int i = countStart; i < countEnd; i++) {
        code(i);
      }
    });
  }

  if (wait) {
    waitForLoopJobs();
  }
}

void doJobs() {

  // sortByPriority();

  for (int i = 0; i < threads.size(); i++) {
    threads[i] = std::thread(createWorkerThread);
    std::cout << "thread : " << " started" << std::endl;
  }
}

void waitAllJobs() {
  std::cout << "wait" << std::endl;
  int threadsLeft = usableThreads - idleThreads;
  if (threadsLeft == 0) {
    std::cout << "threads are done" << std::endl;
    return;
  } else {
    while (threadsLeft != 0) {
      threadsLeft = usableThreads - idleThreads;
      // std::cout << "watint :" << std::endl;
    }
    std::cout << "Threads are done" << std::endl;
  }
}

void shutdownJobsSystem() {

  shouldShutDown = true;
  sem.release(usableThreads);

  std::cout << "Shuting down" << std::endl;

  for (auto &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}
