#include "cmdparser.hpp"
#include "graph.h"
#include "simd_dijkstra.h"

#include <iostream>
#include <string>

void configure_parser(cli::Parser &parser) {
  parser.set_required<std::string>("i", "input_graph",
                                   "Input graph in DIMACs format.");
  parser.set_optional<bool>("s", "show_stats", false,
                            "Show statistics about the computed hub labels.");
};

int main(int argc, char *argv[]) {
  cli::Parser parser(argc, argv, "Arc-Flags SIMD");
  configure_parser(parser);
  parser.run_and_exit_if_error();

  const std::string input = parser.get<std::string>("i");
  const bool showStats = parser.get<bool>("s");

  Graph graph;
  graph.readDimacs(input);

  if (showStats) {
    graph.showStats();
  }

  HWY_NAMESPACE::SIMDDijkstra dijk(graph);

  dijk.run({0, 1, 2, 3, 4, 5, 6, 7});

  for (Vertex v = 0; v < graph.numVertices(); ++v) {
    dijk.printVertex(v);
  }
  return 0;
}
