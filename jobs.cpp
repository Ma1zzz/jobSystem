#include <chrono>
#include "jobs.h"
#include <iostream>
#include <ranges>
#include <vector>
#include <thread>
#include <unistd.h>


 struct jobData
{
    void* function;
    void* dependency;
    int priority;
};

static std::vector<jobData> jobs;

static std::vector<std::thread> threads;

static std::vector<bool> freeThreads;

static int usableThreads;

void reqJobs(void* func, void* dep, int pri)
{
    jobs.push_back({func, dep, pri});
}

void initJobsSystem()
{

    //usableThreads = std::thread::hardware_concurrency();
    usableThreads = 2;

    std::cout << " total threads: " << usableThreads << std::endl;


    usableThreads -= 1;

    if (usableThreads > 4) usableThreads-=1;

    std::cout << " Threads program will use : " << usableThreads << std::endl;

    threads.resize(usableThreads);
    freeThreads.resize(usableThreads, true);
}


static void assignThreadToJob(int threadID, int jobID = 0)
{

    void(*func)() = (void(*)())jobs[jobID].function;
    //jobs.erase(jobs.begin());
    func();
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

static int findFreeThread()
{

    bool isThereAFreeThread = true;
    for (int i = 0; i < usableThreads; i++)
    {
        if (freeThreads[i] == true)
        {
            return i;
        }
    }

    while (true)
    {

        usleep(1000);
        for (int i = 0; i < threads.size(); i++)
        {
            if (freeThreads[i] == true)
            {
                threads[i].join();

                return i;
            }

        }
    }
}

void doJobs()
{
    int jobsToDo = jobs.size();

    for(int jobNumber = 0; jobNumber < jobsToDo; jobNumber++)
    {
        int threadID = findFreeThread();

        freeThreads[threadID] = false;
        threads[threadID] = std::thread(assignThreadToJob, threadID, jobNumber);
    }

    /*for (int threadID = 0; threadID < usableThreads; ++threadID)
    {
        if (freeThreads[threadID] == true && jobNumber < jobsToDo)
        {
            freeThreads[threadID] = false;
            threads[threadID] = std::thread(assignThreadToJob, threadID, jobNumber);

            jobNumber++;
        }
    }*/

    waitAllJobs();
}

