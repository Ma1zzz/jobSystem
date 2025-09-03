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
};

static std::condition_variable threadCondition;
static std::condition_variable isWorkDone;

static bool shouldShutDown = false;

static std::mutex jobsQueueMutex;
static std::mutex mutex;

static std::queue<std::function<void()>> loopJobs;

static std::queue<jobData>& getJobQueue() {
    static std::queue<jobData> jobQueue;
    return jobQueue;
}

static std::atomic<int> loopJobsCount = 0;
static std::atomic<int> loopJobsLeftToComplete = 0;

static std::vector<std::thread> threads;

static std::atomic<int> idleThreads;
//static std::atomic<int> usableThreads;
static int usableThreads;


void reqJobs(std::function<void()> func, std::function<void()> dep, int pri)
{
    std::unique_lock<std::mutex> lock(jobsQueueMutex);
    getJobQueue().push({func, dep, pri});
    lock.unlock();
    threadCondition.notify_one();
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

        idleThreads++;

        threadCondition.wait(lock, []{
            return !getJobQueue().empty() || !loopJobs.empty() || shouldShutDown; });
        if (shouldShutDown)
        {
            //std::cout <<"\n shutting down" << std::endl;
            break;
        }
        idleThreads--;

        if (!loopJobs.empty())
        {
            auto loopJob = loopJobs.front();
            loopJobs.pop();
            lock.unlock();
            loopJob();
            isWorkDone.notify_one();
            continue;
        }


        jobData jobdata = getJobQueue().front();
        auto job= getJobQueue().front().function;
        auto dep= jobdata.dependency;

        getJobQueue().pop();
        lock.unlock();

        if(dep != nullptr)
        {
            dep();
        }
        job();
        isWorkDone.notify_one();
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


static void submitloopJob(std::function<void()> func)
{
    std::unique_lock<std::mutex> lock(jobsQueueMutex);
    loopJobs.push(func);
    lock.unlock();
    threadCondition.notify_one();
}

static void waitForLoopJobs()
{
    while (true)
    {
        jobsQueueMutex.lock();
        if (loopJobs.empty())
        {
            jobsQueueMutex.unlock();
            break;
        }
       // std::cout << "working on loop jobs " <<  std::endl;
        //std::unique_lock<std::mutex> lock(jobsQueueMutex);
        auto job = loopJobs.front();
        loopJobs.pop();
        jobsQueueMutex.unlock();
        job();
    }
}


void parallelLoop(int stat, int end, std::function<void(int)> code, int jobsToCreate, bool wait)
{
    int count = end - stat;
    int remainder =  count % jobsToCreate;
    count -= remainder;
    int loopChunk= (count / (jobsToCreate));
    loopJobsCount += jobsToCreate;
    //loopJobsLeftToComplete += jobsToCreate;


    for (int job = 0; job < jobsToCreate; job++)
    {
        int countStart = loopChunk * job + stat;
        int countEnd = loopChunk + countStart;

        if (job == jobsToCreate - 1) countEnd += remainder;

        submitloopJob([code, countStart, countEnd]() {
            for (int i = countStart; i < countEnd; i++) {
                code(i);
            }
        });
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

void waitAllJobs()
{
    if (idleThreads == usableThreads)
    {
        return;
    }
    else
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::cout << "idle Threads: " << idleThreads << std::endl;
        isWorkDone.wait(lock, []{ return idleThreads == usableThreads; });

        std::cout << "all threads are idle" << std::endl;
    }
}

void shutdownJobsSystem()
{

    shouldShutDown = true;
    threadCondition.notify_all(); // så vækker vi dem så de break;

    //std::cout << "Shuting down" << std::endl;

    for (auto& thread : threads)
    {
        thread.join();
    }
}