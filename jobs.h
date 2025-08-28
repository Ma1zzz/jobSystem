#pragma once
#define DEFINE_JOB(func, dep, pri) \
    static JobRegistrar func##_registrar((void*)func, dep, pri);


 void reqJobs(void* func = nullptr, void* dep = nullptr, int pri = 0);

class JobRegistrar {
public:
    JobRegistrar(void* func, void* dep = nullptr, int pri = 0)
    {
        reqJobs(func, dep, pri);
    }
};


void doJobs();



