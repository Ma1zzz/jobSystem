#include <chrono>
#include <condition_variable>
#include "jobs.h"
#include <functional>
#include <iostream>
#include <queue>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <algorithm>


struct jobData
{
    std::function<void()> function;
    std::function<void()> dependency;
    int priority;
    bool isLoopJob = false;
};

static std::condition_variable loopCompletionCondition;
static std::condition_variable threadCondition;

static bool shouldShutDown = false;

static std::mutex jobsQueueMutex;


static std::queue<jobData>& getJobQueue() {
    static std::queue<jobData> jobQueue;
    return jobQueue;
}

static std::atomic<int> loopJobsCount = 0;
static std::atomic<int> loopJobsLeftToComplete = 0;

static std::vector<std::thread> threads;

static int usableThreads;

void reqJobs(std::function<void()> func, std::function<void()> dep, int pri)
{
    std::unique_lock<std::mutex> lock(jobsQueueMutex);
    getJobQueue().push({func, dep, pri, false});
    lock.unlock();
}

static void sortByPriority()
{

    std::vector<jobData> jobVector;
    std::unique_lock<std::mutex> lock(jobsQueueMutex);

    int length = getJobQueue().size();

    for (int i = 0; i < length; i++)
    {
        jobVector.push_back(getJobQueue().front());
        getJobQueue().pop();
    }

    std::sort(jobVector.begin(), jobVector.end(), [](const jobData& a, const jobData& b) {
        return a.priority < b.priority;
    });


    for (int i = 0; i < jobVector.size(); i++)
    {
        getJobQueue().push(jobVector[i]);
    }
    lock.unlock();
}

static void createWorkerThread()
{
    while (!shouldShutDown)
    {
        std::unique_lock<std::mutex> lock(jobsQueueMutex);


        threadCondition.wait(lock, []{ return !getJobQueue().empty() || shouldShutDown; });
        if (shouldShutDown) break;

        jobData jobdata = getJobQueue().front();
        auto job= getJobQueue().front().function;
        auto dep= jobdata.dependency;

        if (jobdata.isLoopJob)
        {
            --loopJobsCount;
        }


        getJobQueue().pop();
        lock.unlock();

        if(dep != nullptr) dep();
        job();

        if (jobdata.isLoopJob)
        {
            --loopJobsLeftToComplete;
        }
    }
}

void initJobsSystem()
{
    usableThreads = std::thread::hardware_concurrency();
    //usableThreads = 6; // for testing

    if(usableThreads == 1) throw std::runtime_error("Not enough threads for multithreading.");
    std::cout << " total threads: " << usableThreads << std::endl;


    usableThreads -= 1;

    if (usableThreads > 4) usableThreads-=1;

    std::cout << " Threads program will use : " << usableThreads << std::endl;

    threads.resize(usableThreads);
   // freeThreads.resize(usableThreads, true);
    shouldShutDown = false;
}



static void submitloopJob(std::function<void()> func, int priority = 0)
{
    std::unique_lock<std::mutex> lock(jobsQueueMutex);
    getJobQueue().push({func, nullptr, priority, true});
    lock.unlock();

    sortByPriority();
    threadCondition.notify_one();
}

static void waitForLoopJobs()
{
    //std::unique_lock<std::mutex> lock(jobsQueueMutex);
    jobsQueueMutex.lock();

    while (loopJobsCount > 0)
    {
       // std::cout << "working on loop jobs " <<  std::endl;
        //std::unique_lock<std::mutex> lock(jobsQueueMutex);
        auto job = getJobQueue().front();
        getJobQueue().pop();
        --loopJobsCount;
        --loopJobsLeftToComplete;
        jobsQueueMutex.unlock();
        job.function();

    }

    while (loopJobsLeftToComplete != 0)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }


}


void parallelLoop(int stat, int end, std::function<void(int)> code, int jobsToCreate, bool wait)
{
    int count = end - stat;
    int remainder =  count % jobsToCreate;
    count -= remainder;
    int loopChunk= (count / (jobsToCreate));
    loopJobsCount += jobsToCreate;
    loopJobsLeftToComplete += jobsToCreate;


    for (int job = 0; job < jobsToCreate; job++)
    {
        int countStart = loopChunk * job + stat;
        int countEnd = loopChunk + countStart;

        if (job == jobsToCreate - 1) countEnd += remainder;

        submitloopJob([code, countStart, countEnd]() {
            for (int i = countStart; i < countEnd; i++) {
                code(i);
            }
        }, 0);
    }

    if (wait)
    {
        waitForLoopJobs();
    }
}

void doJobs()
{

    sortByPriority();

    for (int i = 0; i < threads.size(); i++)
    {
        threads[i] = std::thread(createWorkerThread);
    }


    std::unique_lock<std::mutex> lock(jobsQueueMutex);
    int queueSize = getJobQueue().size();
    lock.unlock();;
    for (int i = 0; i < queueSize; i++)
    {
        threadCondition.notify_one();
    }
}

void shutdownJobsSystem()
{
    jobsQueueMutex.lock();
    int queueSize = getJobQueue().size();
    jobsQueueMutex.unlock();


    while (queueSize > 0)
    {
        jobsQueueMutex.lock();
        queueSize = getJobQueue().size();
        jobsQueueMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    shouldShutDown = true;
    threadCondition.notify_all(); // så vækker vi dem så de break;

    for (auto& thread : threads)
    {
        thread.join();
    }
}