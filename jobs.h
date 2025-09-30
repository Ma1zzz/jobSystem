#pragma once
#include <functional>
#define DEFINE_JOB(func, dep, ...)                                             \
  static JobRegistrar func##_registrar([]() { func(__VA_ARGS__); }, dep);

void reqJobs(std::function<void()> func, std::function<void()> dep = nullptr);

void parallelLoop(int start, int end, std::function<void(int)> code,
                  int jobsToCreate = 4, bool wait = true);

class JobRegistrar {
public:
  JobRegistrar(std::function<void()> func,
               std::function<void()> dep = nullptr) {
    reqJobs(func, dep);
  }
};

void initJobsSystem();

void doJobs();

void waitAllJobs();

void shutdownJobsSystem();
