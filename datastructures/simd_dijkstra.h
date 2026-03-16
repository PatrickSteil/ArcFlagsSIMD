#pragma once

#include <hwy/highway.h>
#include <vector>

#include "generation_checker.h"
#include "graph.h"
#include "priority_queue.h"
#include "types.h"

class SIMDDijkstra {
private:
  static constexpr HWY_FULL(uint32_t) d{};
  static constexpr size_t L = Lanes(d);
  using Vec = hwy::N_AVX2::Vec<decltype(d)>;

  const Graph &g;
  std::vector<Vec> dist;
  PriorityQueue<> q;
  GenerationChecker<> seen;

public:
  explicit SIMDDijkstra(const Graph &g)
      : g(g), dist(g.numVertices()), q(g.numVertices()), seen(g.numVertices()) {
  }

  void run(const std::vector<Vertex> &sources) {
    if (sources.size() > L) {
      std::cout << "Only start a run with L " << (int)L << " many sources!\n";
      return;
    }
    q.reset();
    seen.reset();

    alignas(64) uint32_t init[L];
    for (size_t i = 0; i < L; i++) {
      init[i] = INF;
    }

    for (std::size_t i = 0; i < sources.size(); ++i) {
      StopId s = sources[i];
      init[i] = 0;
      dist[s] = Load(d, init);
      init[i] = INF;

      seen.mark(s);

      uint32_t key = ReduceMin(d, dist[s]);
      q.push(s, key);
    }

    while (!q.empty()) {
      const auto [_, u] = q.pop();
      relax_edges(u);
    }
  }

  void relax_edges(const Vertex u) {
    Vec du = dist[u];

    g.relaxAllEdges(u, [&](const Vertex, const Vertex to, const Time weight) {
      Vec w = Set(d, weight);
      Vec candidate = Add(du, w);

      if (!seen.isMarked(to)) {
        seen.mark(to);
        dist[to] = Set(d, INF);
      }

      Vec old = dist[to];
      Vec updated = Min(old, candidate);

      if (!AllTrue(d, Eq(old, updated))) {
        dist[to] = updated;

        uint32_t key = ReduceMin(d, updated);
        q.push(to, key);
      }
    });
  }

  Vec getDistance(const Vertex v) const {
    assert(v != noVertex);
    assert(v < dist.size());

    return dist[v];
  }

  void printVec(const Vec &v, const std::string &name = "") const {
    alignas(64) uint32_t tmp[L];
    Store(v, d, tmp);

    if (!name.empty())
      std::cout << name << " = ";

    std::cout << "[";
    for (size_t i = 0; i < L; ++i) {
      if (tmp[i] >= INF)
        std::cout << "INF";
      else
        std::cout << tmp[i];

      if (i + 1 < L)
        std::cout << ", ";
    }
    std::cout << "]\n";
  }

  void printVertex(Vertex v) const {
    std::cout << "Vertex " << v << " ";
    printVec(dist[v], "dist");
  }

  void printRelax(Vertex from, Vertex to, const Vec &fromDist,
                  const Vec &weight, const Vec &candidate, const Vec &oldDist,
                  const Vec &updated) const {

    std::cout << "\nRELAX " << from << " -> " << to << "\n";

    printVec(fromDist, "dist[from]");
    printVec(weight, "weight");
    printVec(candidate, "candidate");
    printVec(oldDist, "old");
    printVec(updated, "updated");
  }
};
