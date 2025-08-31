#pragma once
#include <functional>
#define DEFINE_JOB(func, dep, pri, ...) \
static JobRegistrar func##_registrar([](){ func(__VA_ARGS__); }, dep, pri);



void reqJobs(std::function<void()> func, void* dep = nullptr, int pri = 0);

void parallelLoop(int start , int end, std::function<void(int)> code, int jobsToCreate = 4, bool wait = true);

class JobRegistrar {
public:
    JobRegistrar(std::function<void()> func, void* dep = nullptr, int pri = 0)
    {
        reqJobs(func, dep, pri);
    }
};


void initJobsSystem();

void doJobs();

void shutdownJobsSystem();