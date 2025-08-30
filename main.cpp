#include <iostream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "jobs.h"
#include "chrono"


 static unsigned int xorshift32(unsigned int *state) {
     unsigned int x = *state;
     x ^= x << 13;
     x ^= x >> 17;
     x ^= x << 5;
     return *state = x;
}

int roll() {


    long long count[6] = {0};
    //     unsigned int state = GetTickCount();
    auto now = std::chrono::steady_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    unsigned int state = static_cast<unsigned int>(millis);


    for (int i = 0 ; i<1000000000 ; i++ )
    {
         count[xorshift32(&state) % 6]++;
    }

   /* parallelLoop(1000000000, [&count, &state
    ]
    {
        count[xorshift32(&state) % 6]++;
    }, 4);*/


     printf("\nOne: %lld\nTwo: %lld\nThree: %lld\nFour: %lld\nFive: %lld\nSix: %lld", count[0], count[1], count[2], count[3], count[4], count[5]);

     //printf("\nElapsed time: %lld Seconds & %lld Milliseconds", elapsed/1000, elapsed%1000);
     return 0;
}

//DEFINE_JOB(roll, nullptr, 1)


void printHello()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello, nullptr, 1)
void printHello2()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello2, nullptr, 1)
void printHello3()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello3, nullptr, 1)
void printHell4o()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHell4o, nullptr, 1)
void printHello5()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello5, nullptr, 1)
void printHello6()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello6, nullptr, 1)
void printHello7()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello7, nullptr, 1)
void printHello8()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello8, nullptr, 1)
void printHello9()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello9, nullptr, 1)
void printHello10()
{
    std::cout << "no way this works" << std::endl;
}

DEFINE_JOB(printHello10, nullptr, 1)


int main()
{
    initJobsSystem();
    auto start = std::chrono::high_resolution_clock::now();

   // doJobs();
    //auto start = std::chrono::high_resolution_clock::now();
    //PrimeCalculation();
    //roll();


   /* parallelLoop(4, []()
    {
        std::cout << "no way this works" << std::endl;

    }, 4);*/

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "milliseconds : " << duration.count() << std::endl;

    return 0;
}




