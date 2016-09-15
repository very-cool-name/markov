#include "markov.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

int main() {
  std::string sentence {"The quick brown fox jumps over lazy fox man"};
  std::fstream in("input.txt");
  markov::Chain chain(in);
  chain.Show(std::cout);
}
