#include <chrono>
#include "jobs.h"
#include <iostream>
#include <ranges>
#include <vector>
#include <mutex>
#include <thread>
#include <unistd.h>


 struct jobData
{
    std::function<void()> function;
    void* dependency;
    int priority;
};

struct loopJob
        {
    std::function<void()> function;

};

std::queue<jobData> jobQueue;

static std::vector<jobData> jobs;

static std::vector<loopJob> loopJobs;

static std::vector<std::thread> threads;

static std::vector<std::atomic<bool>> freeThreads;
//static std::vector<bool> freeThreads;

std::mutex freeThreadsMutex;

static int usableThreads;

void reqJobs(std::function<void()> func, void* dep, int pri)
{
    jobQueue.push({func, dep, pri});
    //jobs.push_back({func, dep, pri});
}


void initJobsSystem()
{

    usableThreads = std::thread::hardware_concurrency();
    //usableThreads = 2;


    std::cout << " total threads: " << usableThreads << std::endl;




    usableThreads -= 1; // vi gemmer lige en til systemet

    usableThreads -=1; // her tager vi en mere så vi har plads til vores main thread

    std::cout << " Threads program will use : " << usableThreads << std::endl;

    threads.resize(usableThreads);
    //reeThreads.resize(usableThreads, true);
}


static void assignThreadToJob(int threadID, std::function<void()> jobToDo)
{
    jobToDo();
    //std::lock_guard<std::mutex> lock(freeThreadsMutex);
    freeThreads[threadID] = true;
}


static void waitAllJobs()
{
    for (auto& thread : threads)
    {
        if (thread.joinable())  // This check prevents the error
        {
            thread.join();
        }

    }
}


static int checkForFreeThread()
{
   // std::lock_guard<std::mutex> lock(freeThreadsMutex);
    for (int i = 0; i < threads.size(); i++)
    {
        if (freeThreads[i])
        {
            return i;
        }
    }
    return 999; // just a number for erorr
}

static int waitForThreadToFinish()
{
    //std::lock_guard<std::mutex> lock(freeThreadsMutex);
    for (int i = 0; i < threads.size(); i++)
    {
        if (freeThreads[i])
        {
            std::cout << "test " << std::endl;

            //if (threads[i].joinable())
            threads[i].join();

            return i;
        }
    }

    return 999;
}

static int findFreeThread()
{

    int returnId = checkForFreeThread();

    if (returnId != 999)
    {
        return returnId;
    }

    while (true)
    {
        usleep(1000);

        returnId = waitForThreadToFinish();
        if (returnId != 999) return returnId;
    }
}



static void assignThreadJob();

void doJobs()
{
    unsigned int totalJobs = jobs.size() + loopJobs.size();
    if(totalJobs == 0) return;

std::cout << totalJobs << std::endl;

    for (int i = 0; i < totalJobs; ++i)
    {
        for (int j = 0; j < loopJobs.size(); ++j)
        {
            int threadID = findFreeThread();

            //std::lock_guard<std::mutex> lock(freeThreadsMutex);
            freeThreads[threadID] = false;
            threads[threadID] = std::thread(assignThreadToJob, threadID, loopJobs[j].function);
        }

        for (int y = 0; y < jobs.size(); y++)
        {
            int threadID = findFreeThread();

           // std::lock_guard<std::mutex> lock(freeThreadsMutex);
            freeThreads[threadID] = false;

            threads[threadID] = std::thread(assignThreadToJob, threadID, jobs[y].function);
        }
    }

    waitAllJobs();
}




static void functionToParral(int loopPrJob, std::function<void()> func)
{
    for (int i = 0; i < loopPrJob; ++i)
    {
        func();
    }
}


void parallelLoop(int count, std::function<void()> code, int jobsToCreate)
{

}
