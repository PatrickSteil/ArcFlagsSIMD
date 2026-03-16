#pragma once

/// https://mhdm.dev/posts/sb_lower_bound/

#include <algorithm>
#include <cstddef>
#include <iterator>

template <typename RandomIt, typename T, typename Compare>
RandomIt sbp_lower_bound(RandomIt first, RandomIt last, const T &value,
                         Compare comp) {
  using value_type = typename std::iterator_traits<RandomIt>::value_type;
  auto length = std::distance(first, last);

  if (length == 0)
    return first;

  // Check if we can safely use pointers
  if constexpr (std::is_pointer_v<RandomIt> ||
                std::is_same_v<RandomIt, value_type *> ||
                std::is_same_v<RandomIt, const value_type *>) {
    // already raw pointers, no cast needed
    auto *ptr_first = &*first;

    constexpr int entries_per_256KB = 256 * 1024 / sizeof(value_type);

    if (length >= entries_per_256KB) {
      constexpr int num_per_cache_line =
          std::max(64 / int(sizeof(value_type)), 1);

      while (length >= 3 * num_per_cache_line) {
        auto half = length / 2;
        __builtin_prefetch(ptr_first + half / 2);
        auto *first_half1 = ptr_first + (length - half);
        __builtin_prefetch(first_half1 + half / 2);

        ptr_first = comp(ptr_first[half], value) ? first_half1 : ptr_first;
        length = half;
      }
    }

    while (length > 0) {
      auto half = length / 2;
      auto *first_half1 = ptr_first + (length - half);
      ptr_first = comp(ptr_first[half], value) ? first_half1 : ptr_first;
      length = half;
    }

    return RandomIt(ptr_first);
  } else {
    // fallback for non-pointer random access iterators
    auto it = first;
    while (length > 0) {
      auto half = length / 2;
      auto mid = std::next(it, half);
      it = comp(*mid, value) ? std::next(it, length - half) : it;
      length = half;
    }
    return it;
  }
}
