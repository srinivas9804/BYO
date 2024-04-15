#include "gbbs/bridge.h"
#include "include/terrace_graph.h"
#include "../run_unweighted.h"
#include <iostream>

template <template <class W> class vertex_type, class W>
struct TerraceWrapper{
  using vertex = vertex_type<W>;
  using weight_type = W;
  static constexpr bool binary = true;
  static_assert(binary);
  using vertex_weight_type = double;
  using edge_type = typename vertex::edge_type;

  size_t M() const { return graph.M(); }

  size_t N() const { return graph.N(); }

  auto degree(size_t i) const { return graph.out_degree(i); }

  template <class F> void map_neighbors(size_t i, F f) const {
    W empty_weight = W();
    graph.map_neighbors(i,f,empty_weight);
  }
  template <class F> void map_neighbors_early_exit(size_t i, F f) const {
    W empty_weight = W();
    graph.map_neighbors(i,f,empty_weight);
  }
  template <class F> void parallel_map_neighbors(size_t i, F f) const {
    W empty_weight = W();
    graph.map_neighbors(i,f,empty_weight);
  }

  template <class F>
  void parallel_map_neighbors_early_exit(size_t i, F f) const {
    W empty_weight = W();
    graph.map_neighbors(i,f,empty_weight);
  }
  TerraceWrapper() :  vertex_weights(nullptr){}

  TerraceWrapper(auto *v_data, size_t n, size_t m,
                std::function<void()> _deletion_fn, edge_type *_e0,
                vertex_weight_type *_vertex_weights = nullptr) 
                : vertex_weights(_vertex_weights), deletion_fn(_deletion_fn), graph(n) {
      printf("INITIALIZE TERRACE GRAPH WITH NODES %lu, EDGES %lu\n", n, m);
      printf("nodes in graph = %u\n", graph.N());
      printf("edges in graph = %lu\n", graph.M());  

      std::vector<std::tuple<int,int,int>> edgesMap(m);

      printf("convert to edge list\n");
      gbbs::parallel_for(0, n, [&](uint32_t i) {
        for (size_t j = v_data[i].offset; j < v_data[i].offset + v_data[i].degree;
            j++) {
          int src = i;
          int dest = std::get<0>(_e0[j]);
          edgesMap[j] = {src,dest,0};
          // std::cout << "src,dest:" << srcs[j] << "," << dests[j] << std::endl; 
          // TODO: weights if necessary
        }
      });
      graph.build_batch(edgesMap,true);
      printf("nodes in graph = %u\n", graph.N());
      printf("edges in graph = %lu\n", graph.M());  
    }


  // Graph Data
  TerraceGraph graph;
  vertex_weight_type *vertex_weights;
  // called to delete the graph
  std::function<void()> deletion_fn;
};

using graph_impl = TerraceWrapper<gbbs::symmetric_vertex, gbbs::empty>;
using graph_api = gbbs::full_api;
using graph_t = gbbs::Graph<graph_impl, /* symmetric */ true, graph_api>;

int main(int argc, char *argv[]) {
  gbbs::commandLine P(argc, argv, " [-s] <inFile>");
  char *iFile = P.getArgument(0);
  bool symmetric = P.getOptionValue("-s");
  bool compressed = P.getOptionValue("-c");
  bool binary = P.getOptionValue("-b");
  bool mmap = P.getOptionValue("-m");
  gbbs::run_all_options options;
  options.dump = P.getOptionValue("-d");
  options.rounds = P.getOptionLongValue("-rounds", 3);
  options.src = static_cast<gbbs::uintE>(P.getOptionLongValue("-src", 0));

  std::cout << "### Graph: " << iFile << std::endl;
  if (compressed) {
    std::cerr << "does not support compression\n";
    return -1;
  } else {
    if(symmetric){
      auto G = gbbs::gbbs_io::read_unweighted_symmetric_graph<graph_t>(
          iFile, mmap, binary);
      // run_all<true>(G, options);
    }
    else{
      TerraceGraph graph(iFile,false);
      // gbbs::BFS_runner(graph, 1, 5, true);
      // run_all<true>(graph, options);
    }
  }
  return 1;
}