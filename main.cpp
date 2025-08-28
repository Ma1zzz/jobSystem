#include <iostream>
#include "jobs.h"

void printHello()
{
    printf("Hello!\n");
}
DEFINE_JOB(printHello, nullptr, 0)

int main()
{
    doJobs();



    return 0;
}
