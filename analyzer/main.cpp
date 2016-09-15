#include <sstream>
#include <iostream>
#include <fstream>

#include "cereal/archives/json.hpp"

#include "markov_chain/markov.hpp"

int main() {
  std::fstream in("input.txt");
  markov::Chain chain(in);
  cereal::JSONOutputArchive output(std::cout);
  output(chain);  
}
