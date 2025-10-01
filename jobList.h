#pragma once
#include <cmath>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
template <typename t> class myVector {
private:
  t *dataType;
  size_t amoutDataStored;
  size_t currentSize;
  size_t capacity;
  size_t pageSize;
  void *data;

public:
  myVector() {

    data = mmap(nullptr, 1024, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    pageSize = sysconf(_SC_PAGESIZE);
    capacity = pageSize;
    amoutDataStored = 0;
    currentSize = amoutDataStored * sizeof(*dataType);
    std::cout << "Size " << sizeof(*dataType) << std::endl;
    std::cout << "Page size : " << pageSize << std::endl;
  }
  ~myVector() { munmap(data, pageSize); }

  void resize(int amount) {
    float bytesNeeded = amount * sizeof(*dataType);
    int neededPages = static_cast<int>(std::ceil((bytesNeeded / pageSize)));

    void *newData =
        mmap(nullptr, (neededPages * pageSize), PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // kopir lige det gamle data over
    data = newData;
    capacity = neededPages * pageSize;
    std::cout << "new capacity in bytes : " << capacity << std::endl;
  }

  void add(t &&input) {
    int thisDataNumber;

    /*jobda *test = (jobda *)input;

    test->job();*/

    if (currentSize != 0) {
      int currentDataNumber = currentSize / sizeof(*dataType);
      thisDataNumber = currentDataNumber + 1;
    } else {
      thisDataNumber = 1;
    }

    currentSize += sizeof(*dataType);
    thisDataNumber--;

    void *ptr = (char *)data + (sizeof(*dataType) * (thisDataNumber));

    new (ptr) t(std::move(input));
    // std::memcpy(ptr, input, sizeof(*dataType));
  }

  void *getVal(int number) {

    int bytesToShift = number * sizeof(*dataType);

    void *ptr = (char *)data + bytesToShift;
    return ptr;
  }

  void clear();
};
