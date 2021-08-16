#include "results.hpp"

using namespace std;
using namespace caf;
using namespace std::chrono;

value::value() : 
  result(0),
  tree_id(""),
  node_id(-1) {
    this->ts = system_clock::now();
  }

value::value(double res) : 
  result(res),
  tree_id(""),
  node_id(-1) {
    this->ts = system_clock::now();
  }

value::value(double result, string tree, int id) :
  result(result),
  tree_id(tree),
  node_id(id) {
    this->ts = system_clock::now();
  }



