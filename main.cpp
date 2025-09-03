#include <iostream>
#include <cmath>
#include <unistd.h>

#include "jobs.h"
#include "chrono"
#include "printf.h"

int testVarible;

void PrimeCalculation()
{
    std::atomic<int> count = 0;
    parallelLoop(2, 500000, [&count](int n ) {

        bool isPrime = true;
        for(int i = 2; i <= sqrt(n); i++) {
            if(n % i == 0) {
                isPrime = false;
                break;
            }
        }
        if(isPrime) count++;
    },5, true );

    std::cout << "Found " << count << " primes\n";

}

void PrimeCalculation2()
{
    std::atomic<int> count = 0;
    parallelLoop(2, 500000, [&count](int n ) {

        bool isPrime = true;
        for(int i = 2; i <= sqrt(n); i++) {
            if(n % i == 0) {
                isPrime = false;
                break;
            }
        }
        if(isPrime) count++;
    },5, true );

    std::cout << "Found " << count << " primes\n";

}
void PrimeCalculation3() {
    int count = 0;
    for(int n = 2; n < 500000; n++) {
        bool isPrime = true;
        for(int i = 2; i <= sqrt(n); i++) {
            if(n % i == 0) {
                isPrime = false;
                break;
            }
        }
        if(isPrime) count++;
    }
    std::cout << "Found " << count << " primes\n";
}
void PrimeCalculation4() {
    int count = 0;
    for(int n = 2; n < 500000; n++) {
        bool isPrime = true;
        for(int i = 2; i <= sqrt(n); i++) {
            if(n % i == 0) {
                isPrime = false;
                break;
            }
        }
        if(isPrime) count++;
    }
    std::cout << "Found " << count << " primes\n";
}


void test()
{
    DEFINE_JOB(PrimeCalculation, nullptr, 1)

}

void printNumber(int number)
{
    std::cout << number << std::endl;;
}

//DEFINE_JOB(printNumber, nullptr, 1, testVarible)

//DEFINE_JOB(PrimeCalculation, printHello, 1)
//DEFINE_JOB(PrimeCalculation2, nullptr, 1)

int main()
{
    initJobsSystem();
    auto start = std::chrono::high_resolution_clock::now();
    doJobs();


    //auto start = std::chrono::high_resolution_clock::now();
    PrimeCalculation();
    //PrimeCalculation3();

    waitAllJobs();
    std::cout << "er noget her til " << std::endl;

    shutdownJobsSystem();
    //std::cout << "number : " << count << std::endl;
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "milliseconds : " << duration.count() << std::endl;

    return 0;
}