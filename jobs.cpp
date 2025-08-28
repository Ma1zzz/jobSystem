#include "jobs.h"
#include <iostream>


 struct jobData
{
    void* function;
    void* dependency;
    int priority;
};

static std::vector<jobData> jobs;



void reqJobs(void* func, void* dep, int pri)
{
    jobs.push_back({func, dep, pri});
}

void doJobs() {

    for (jobData job: jobs)
    {

        void(*func)() = (void(*)())job.function;
        func();
    }
}
