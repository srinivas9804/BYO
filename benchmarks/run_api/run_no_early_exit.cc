

// Usage:
// numactl -i all ./run_unweighted -src 10012 -s -m -rounds 3 twitter_SJ
// flags:
//   required:
//     -src: the source to compute the BFS from
//   optional:
//     -rounds : the number of times to run the algorithm
//     -c : indicate that the graph is compressed
//     -m : indicate that the graph should be mmap'd
//     -s : indicate that the graph is symmetric
//     -d : dump the output arrays to files, useful for debugging

#define GRAPH_API_DEGREE true
#define GRAPH_API_M true
#define GRAPH_API_PARALLEL_MAP true
#define GRAPH_API_MAP_EARLY_EXIT false
#define GRAPH_API_PARALLEL_MAP_EARLY_EXIT false
#define GRAPH_API_STORE_M true
#define GRAPH_API_STORE_DEGREES true

#define UNWEIGHTED_SYM_GRAPH_IMPL                                              \
  gbbs::graph_implementations::symmetric_graph<gbbs::symmetric_vertex,         \
                                               gbbs::empty, false>

#include "../run_unweighted.h"

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
    if (symmetric) {
      auto G = gbbs::gbbs_io::read_compressed_symmetric_graph<gbbs::empty>(
          iFile, mmap);
      run_all<true>(G, options);
    } else {
      auto G = gbbs::gbbs_io::read_compressed_asymmetric_graph<gbbs::empty>(
          iFile, mmap);
      run_all<false>(G, options);
    }
  } else {
    if (symmetric) {
      auto G =
          gbbs::gbbs_io::read_unweighted_symmetric_graph<unweighted_sym_graph>(
              iFile, mmap, binary);
      run_all<true>(G, options);
    } else {
      auto G = gbbs::gbbs_io::read_unweighted_asymmetric_graph<
          unweighted_asym_graph>(iFile, mmap, binary);
      run_all<false>(G, options);
    }
  }
  return 1;
}