#include "jobs.h"
#include <iostream>
#include <thread>

 struct jobData
{
    void* function;
    void* dependency;
    int priority;
};

static std::vector<jobData> jobs;

static std::vector<std::thread> threads;

static int usableThreads;

void reqJobs(void* func, void* dep, int pri)
{
    jobs.push_back({func, dep, pri});
}

void initJobsSystem()
{

    usableThreads = std::thread::hardware_concurrency();

    std::cout << " total threads: " << usableThreads << std::endl;


    usableThreads -= 1;

    if (usableThreads > 4) usableThreads-=1;

    std::cout << " Threads program will use : " << usableThreads << std::endl;

    threads.resize(usableThreads);
}


static void assignJobs(int threadID)
{
    void(*func)() = (void(*)())jobs[threadID].function;
    func();
    jobs.erase(jobs.begin());
}

void doJobs()
{
    for (int i = 0; i < usableThreads; ++i)
    {
        assignJobs(i);
    }
}

