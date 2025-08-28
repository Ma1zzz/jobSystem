#include <iostream>
#include <cmath>
#include "jobs.h"
#include "chrono"



void PrimeCalculation() {
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
void PrimeCalculation2() {
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
DEFINE_JOB(PrimeCalculation, nullptr, 0)
DEFINE_JOB(PrimeCalculation2, nullptr, 0)
DEFINE_JOB(PrimeCalculation3, nullptr, 0)
DEFINE_JOB(PrimeCalculation4, nullptr, 0)

int main()
{
    initJobsSystem();
    auto start = std::chrono::high_resolution_clock::now();
    doJobs();
    //auto start = std::chrono::high_resolution_clock::now();
    //PrimeCalculation();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "milliseconds : " << duration.count() << std::endl;

    return 0;
}




