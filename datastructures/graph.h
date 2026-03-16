#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "status_log.h"
#include "types.h"

struct Edge {
  Vertex from;
  Vertex to;
  Time weight;

  Edge() = default;
  Edge(Vertex f, Vertex t, Time w) : from(f), to(t), weight(w) {}

  bool operator<(const Edge &other) const {
    return std::tie(from, to, weight) <
           std::tie(other.from, other.to, other.weight);
  }
};

struct Graph {
public:
  std::vector<std::size_t> adj; // size = n+1
  std::vector<Vertex> to;       // size = m
  std::vector<Time> w;          // size = m

  Graph() : adj(1, 0) {}

  Graph(std::size_t n, std::size_t m) : adj(n + 1, 0), to(m), w(m) {}

  std::size_t numVertices() const { return adj.size() - 1; }
  std::size_t numEdges() const { return to.size(); }

  bool isValid(Vertex v) const { return v < numVertices(); }

  std::size_t beginEdge(Vertex v) const {
    assert(isValid(v));
    return adj[v];
  }

  std::size_t endEdge(Vertex v) const {
    assert(isValid(v));
    return adj[v + 1];
  }

  std::size_t degree(Vertex v) const { return endEdge(v) - beginEdge(v); }

  void clear() {
    adj.assign(1, 0);
    to.clear();
    w.clear();
  }

  void buildFromEdgeList(std::vector<Edge> &edges, std::size_t n) {
    clear();

    std::sort(edges.begin(), edges.end());

    adj.assign(n + 1, 0);

    for (const auto &e : edges) {
      assert(e.from < n);
      assert(e.to < n);
      ++adj[e.from + 1];
    }

    for (std::size_t i = 1; i <= n; ++i)
      adj[i] += adj[i - 1];

    to.resize(edges.size());
    w.resize(edges.size());

    std::vector<std::size_t> offset = adj;

    for (const auto &e : edges) {
      std::size_t idx = offset[e.from]++;
      to[idx] = e.to;
      w[idx] = e.weight;
    }
  }

  void readDimacs(const std::string &fileName) {
    StatusLog log("Read from dimacs file");
    clear();

    std::ifstream file(fileName);
    if (!file.is_open())
      throw std::runtime_error("Cannot open file");

    std::string line;
    std::vector<Edge> edges;
    Vertex n = 0;
    std::size_t m = 0;

    while (std::getline(file, line)) {
      if (line.empty() || line[0] == 'c')
        continue;

      if (line[0] == 'p') {
        std::istringstream iss(line);
        std::string tmp;
        iss >> tmp >> tmp >> n >> m;
        edges.reserve(m);
      }

      if (line[0] == 'a') {
        std::istringstream iss(line);
        char a;
        Vertex u, v;
        Time weight;
        iss >> a >> u >> v >> weight;

        edges.emplace_back(u - 1, v - 1, weight);
      }
    }

    buildFromEdgeList(edges, n);
  }

  void writeDimacs(const std::string &fileName) const {
    std::ofstream file(fileName);
    if (!file.is_open())
      throw std::runtime_error("Cannot open file");

    file << "p sp " << numVertices() << " " << numEdges() << "\n";

    for (Vertex u = 0; u < numVertices(); ++u) {
      for (std::size_t i = beginEdge(u); i < endEdge(u); ++i) {
        file << "a " << (u + 1) << " " << (to[i] + 1) << " " << w[i] << "\n";
      }
    }
  }

  Graph reverse() const {
    Graph rev;
    rev.adj.assign(numVertices() + 1, 0);
    rev.to.resize(numEdges());
    rev.w.resize(numEdges());

    for (Vertex u = 0; u < numVertices(); ++u)
      for (std::size_t i = beginEdge(u); i < endEdge(u); ++i)
        ++rev.adj[to[i] + 1];

    for (std::size_t i = 1; i <= numVertices(); ++i)
      rev.adj[i] += rev.adj[i - 1];

    std::vector<std::size_t> offset = rev.adj;

    for (Vertex u = 0; u < numVertices(); ++u) {
      for (std::size_t i = beginEdge(u); i < endEdge(u); ++i) {
        Vertex v = to[i];
        std::size_t idx = offset[v]++;
        rev.to[idx] = u;
        rev.w[idx] = w[i];
      }
    }

    return rev;
  }

  template <typename F> void doForAllEdges(F &&f) const {
    for (Vertex u = 0; u < numVertices(); ++u)
      for (std::size_t i = beginEdge(u); i < endEdge(u); ++i)
        f(u, to[i], w[i]);
  }

  template <typename F> void relaxAllEdges(Vertex u, F &&f) const {
    for (std::size_t i = beginEdge(u); i < endEdge(u); ++i)
      f(u, to[i], w[i]);
  }

  void showStats() const {
    std::size_t minDeg = std::numeric_limits<std::size_t>::max();
    std::size_t maxDeg = 0;
    std::size_t total = 0;
    std::size_t isolated = 0;

    for (Vertex v = 0; v < numVertices(); ++v) {
      std::size_t d = degree(v);
      if (d == 0)
        ++isolated;

      minDeg = std::min(minDeg, d);
      maxDeg = std::max(maxDeg, d);
      total += d;
    }

    double avg = static_cast<double>(total) / numVertices();
    std::cout << "Graph Statistics:" << std::endl;
    std::cout << "\tVertices: " << numVertices() << "\n";
    std::cout << "\tEdges:    " << numEdges() << "\n";
    std::cout << "\tMinDeg:   " << minDeg << "\n";
    std::cout << "\tMaxDeg:   " << maxDeg << "\n";
    std::cout << "\tAvgDeg:   " << avg << "\n";
    std::cout << "\tIsolated: " << isolated << "\n";
  }
};
