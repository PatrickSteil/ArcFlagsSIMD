#pragma once

#include <time.h>
#include <cstdint>

class Timer {
public:
  Timer() noexcept : start_(0), end_(0) {}

  inline void start() noexcept {
    start_ = timestamp();
  }

  inline void restart() noexcept {
    start_ = timestamp();
  }

  inline void stop() noexcept {
    end_ = timestamp();
  }

  inline void reset() noexcept {
    start_ = 0;
    end_ = 0;
  }

  inline uint64_t elapsedNanoseconds() const noexcept {
    return end_ - start_;
  }

  inline double elapsedMicroseconds() const noexcept {
    return (end_ - start_) * 1e-3;
  }

  inline double elapsedMilliseconds() const noexcept {
    return (end_ - start_) * 1e-6;
  }

private:
  static inline uint64_t timestamp() noexcept {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull + ts.tv_nsec;
  }

private:
  uint64_t start_;
  uint64_t end_;
};
