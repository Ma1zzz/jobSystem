#define DEBUGMODE false
#include "jobs.h"
#include "jobList.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <limits.h>
#ifdef __linux__
#include <linux/futex.h>
#elif __APPLE__
#include <dispatch/dispatch.h>
dispatch_semaphore_t sem = dispatch_semaphore_create(0);
#endif
#include <mutex>
#include <queue>
#include <sys/syscall.h>
#include <thread>
#include <vector>

struct jobData {
  std::function<void()> function;
  std::function<void()> dependency;
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

static std::vector<std::vector<jobData>> jobThreadQueueVector;
static std::vector<std::mutex> mutexList;

static std::atomic<int> jobCount = 0;

static std::atomic<int> loopJobsCount = 0;
static std::atomic<int> loopJobsLeftToComplete = 0;

static std::vector<std::thread> threads;

static std::atomic<int> idleThreads;
// static int idleThreads;
static std::atomic<int> usableThreads;
// static int usableThreads;
//

static myVector<jobData> jobDataList;

static std::atomic<int> futexVal = 0;

void reqJobs(std::function<void()> func, std::function<void()> dep) {

  jobData job = {func, dep};
  jobDataList.add(std::move(job));

  // futexVal++;
#ifdef __linux__

  syscall(SYS_futex, &futexVal, FUTEX_WAIT, 1);
#elif __APPLE__
  dispatch_semaphore_signal(sem);
#endif

  ++jobCount;
}

static std::atomic<int> jobNumber = 0;

static void createWorkerThread(int id) {

  while (!shouldShutDown) {

    // std::cout << "jobCount : " << jobCount << std::endl;
    idleThreads++;
    // jobCount
    if (jobCount == 0) {
      for (int i = 0; i < 1000; i++) {
        std::this_thread::yield();
        if (jobCount != 0) {
          break;
        }
      }
#ifdef __linux__
      syscall(SYS_futex, &futexVal, FUTEX_WAIT, 0);
#elif __APPLE__
      // dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
#endif

      // sem.acquire();
    } else {

      futexVal--;
      // sem.acquire();
    }
#ifdef __APPLE__
    // std::cout << "LOL :" << std::endl;
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
#endif

    if (shouldShutDown) {
      // std::cout << "THREAD IS GOING BYE BYE" << std::endl;
      break;
    }

    --jobCount;
    // std::cout << getJobQueue().size() << std::endl;

    idleThreads--;

    // std::unique_lock<std::mutex> lock(jobsQueueMutex);
    // jobsQueueMutex.lock();
    if (!loopJobs.empty()) {
      auto loopJob = loopJobs.front();
      loopJobs.pop();
      jobsQueueMutex.unlock();
      loopJob();
      isWorkDone.notify_one();
      continue;
    }

    void *ptr = jobDataList.getVal(jobNumber.fetch_add(1));
    jobData *jobdata = (jobData *)ptr;

    // std::cout << ptr << std::endl;

    std::function<void()> job = jobdata->function;

    auto dep = jobdata->dependency;

    if (dep != nullptr) {
      dep();
    }

    job();
    // std::cout << "job is done " << std::endl;
  }
  // std::cout << "THREAD WANNA DIEEE" << std::endl;
}

void initJobsSystem() {
  // usableThreads = std::thread::hardware_concurrency();
  jobDataList.resize(50000);
  usableThreads = 6; // for testing
  std::cout << usableThreads << std::endl;
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
  getJobQueue.reserve(500000);

  /* for (int i = 0; i < threads.size(); i++) {
     threads[i] = std::thread(createWorkerThread, i);

     std::vector<jobData> data;
     jobThreadQueueVector.push_back(data);

     std::cout << "thread : " << " started" << std::endl;
   }*/
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

  for (int i = 0; i < threads.size(); i++) {
    threads[i] = std::thread(createWorkerThread, i);

    std::vector<jobData> data;
    jobThreadQueueVector.push_back(data);

    std::cout << "thread : " << " started" << std::endl;
  }
  // probaly gonna remove this
}

void waitAllJobs() {

  int threadsLeft = usableThreads - idleThreads;
  if (threadsLeft == 0) {
    std::cout << "threads are done" << std::endl;
    return;
  } else {
    while (threadsLeft != 0) {
      threadsLeft = usableThreads - idleThreads;
      // std::cout << "watint :" << std::endl;
    }
  }
}

void shutdownJobsSystem() {

  shouldShutDown = true;

#ifdef __APPLE__
  for (int y = 0; y < usableThreads; y++) {

    dispatch_semaphore_signal(sem);
  }
#endif
#ifdef __linux__
  syscall(SYS_futex, &futexVal, FUTEX_WAKE, INT_MAX);
#elif __APPLE__
  // std::cout << "Shuting down" << std::endl;
#endif

  for (int i = 0; i < threads.size(); i++) {
    if (threads[i].joinable()) {
      threads[i].join();
    }
  }
}
