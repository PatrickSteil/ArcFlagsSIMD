/*
 * Licensed under MIT License.
 * Author: Patrick Steil
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include "types.h"

template <uint32_t ARITY = 4, typename Comparator = std::less<uint32_t>>
class PriorityQueue {
private:
  using HeapElement = std::pair<uint32_t, Vertex>;
  const std::uint32_t noIndex = static_cast<std::uint32_t>(-1);

  std::vector<HeapElement> heap;
  std::vector<std::uint32_t> mapper;
  Comparator comp;

public:
  PriorityQueue(size_t size = 0) : heap(), mapper(size, noIndex), comp() {}

  size_t size() const { return heap.size(); }
  bool empty() const { return heap.empty(); }
  size_t capacity() const { return mapper.size(); }

  bool isValid(std::uint32_t i) const { return i < mapper.size(); }

  bool heapPropertyFullfilled() const {
    for (std::uint32_t i = 0; i < size(); ++i) {
      for (std::uint32_t k = 0; k < ARITY; ++k) {
        std::uint32_t child = getChild(i, k);
        if (child < size() && comp(heap[child].first, heap[i].first)) {
          std::cerr << "[FAIL] Heap property violated\n";
          return false;
        }
      }
    }
    return true;
  }

  uint32_t get(Vertex v) const {
    if (mapper[v] == noIndex)
      return uint32_t(-1);
    return heap[mapper[v]].first;
  }

  std::uint32_t getChild(std::uint32_t i, std::uint32_t k) const {
    return i * ARITY + 1 + k;
  }

  std::uint32_t getParentIndex(std::uint32_t i) const {
    return (i - 1) / ARITY;
  }

  void swap(std::uint32_t i, std::uint32_t j) {
    assert(isValid(i) && isValid(j));
    mapper[heap[i].second] = j;
    mapper[heap[j].second] = i;
    std::swap(heap[i], heap[j]);
  }

  void siftDown(std::uint32_t i) {
    assert(isValid(i));

    while (true) {
      std::uint32_t smallest = i;

#pragma GCC unroll
      for (std::uint32_t k = 0; k < ARITY; ++k) {
        std::uint32_t child = getChild(i, k);
        if (child < size() && comp(heap[child].first, heap[smallest].first)) {
          smallest = child;
        }
      }

      if (smallest == i)
        return;

      swap(i, smallest);
      i = smallest;
    }
  }

  void siftUp(std::uint32_t i) {
    assert(isValid(i));
    while (i > 0) {
      std::uint32_t parent = getParentIndex(i);
      if (comp(heap[i].first, heap[parent].first)) {
        swap(i, parent);
        i = parent;
      } else {
        return;
      }
    }
  }

  void push(Vertex v, uint32_t value) {
    assert(isValid(v));

    if (mapper[v] == noIndex) {
      heap.emplace_back(value, v);
      mapper[v] = size() - 1;
      siftUp(size() - 1);
    } else {
      std::uint32_t i = mapper[v];
      if (heap[i].first == value) {
        return;
      }

      if (comp(heap[i].first, value)) {
        heap[i].first = value;
        siftDown(i);
      } else {
        heap[i].first = value;
        siftUp(i);
      }
    }
  }

  std::pair<uint32_t, Vertex> pop() {
    if (empty()) {
      return {0, noVertex};
    }

    auto result = heap[0];
    swap(0, size() - 1);
    heap.pop_back();
    mapper[result.second] = noIndex;

    if (!empty()) {
      siftDown(0);
    }

    assert(heapPropertyFullfilled());
    return result;
  }

  const HeapElement &front() const {
    assert(!empty());
    return heap.front();
  }

  void buildFrom(std::vector<uint32_t> &values) {
    mapper.clear();
    heap.clear();

    if (values.size() == 0)
      return;

    mapper.resize(values.size());
    heap.resize(values.size());

    for (Vertex v = 0; v < values.size(); ++v) {
      heap[v] = std::make_pair(values[v], v);
      mapper[v] = v;
    }
    buildHeapRecursive(0);
    assert(heapPropertyFullfilled());
  }

  void buildHeapRecursive(std::uint32_t i) {
    for (std::uint32_t k = 0; k < ARITY; ++k) {
      std::uint32_t child = getChild(i, k);
      if (child < size())
        buildHeapRecursive(child);
    }
    siftDown(i);
  }

  void reset() {
    std::fill(mapper.begin(), mapper.end(), noIndex);
    heap.clear();
    heap.reserve(capacity());
  }
};

struct MaxHeapComparator {
  bool operator()(uint32_t a, uint32_t b) const { return a < b; }
};
