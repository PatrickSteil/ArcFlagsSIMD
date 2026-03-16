#pragma once

#include <thread>
#include <vector>

template <typename FUNC>
void parallelFor(std::size_t start, std::size_t end, FUNC &&func,
                 std::size_t numThreads = std::thread::hardware_concurrency()) {
  std::vector<std::thread> threads;
  std::size_t chunkSize = (end - start + numThreads - 1) / numThreads;

  for (std::size_t t = 0; t < numThreads; ++t) {
    std::size_t chunkStart = start + t * (chunkSize);
    std::size_t chunkEnd = std::min(chunkStart + chunkSize, end);

    if (chunkStart >= end)
      break;

    threads.emplace_back([=, &func]() {
      for (std::size_t i = chunkStart; i < chunkEnd; ++i) {
        func(t, i);
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }
}
