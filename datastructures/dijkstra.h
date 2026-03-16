#pragma once

#include <vector>

#include "generation_checker.h"
#include "graph.h"
#include "priority_queue.h"
#include "types.h"

class Dijkstra {
private:
  const Graph &g;
  std::vector<Time> dist;
  PriorityQueue<> q;
  GenerationChecker<> seen;

public:
  explicit Dijkstra(const Graph &g)
      : g(g), dist(g.numVertices(), INF), q(g.numVertices()),
        seen(g.numVertices()) {}

  void run(const StopId source) {
    q.reset();
    seen.reset();

    dist[source] = 0;
    seen.mark(source);
    q.push(source, 0);

    while (!q.empty()) {
      const auto [d, u] = q.pop();
      assert(u != noVertex);

      relax_edges(u);
    }
  }

  void relax_edges(const Vertex u) {
    const Time arrivalTime = dist[u];
    g.relaxAllEdges(u, [&](const Vertex, const Vertex to, const Time weight) {
      relax(to, arrivalTime + weight);
    });
  }

  Time getDistance(const Vertex v) const {
    assert(v != noVertex);
    assert(v < dist.size());

    return dist[v];
  }

  inline bool relax(const Vertex to, const Time newTime) {
    bool notFirstTime = seen.isMarked(to);
    dist[to] = notFirstTime ? dist[to] : INF;
    seen.mark(to);

    if (newTime < dist[to]) {
      dist[to] = newTime;
      q.push(to, newTime);

      return true;
    }

    return false;
  }
};
