#include <sstream>
#include <iostream>
#include <fstream>

#include <cereal/archives/json.hpp>

#include <markov_chain/markov.hpp>

int main() {
  {
    std::fstream in("input.txt");
    std::fstream out("output.txt");
    markov::Chain chain(in);
    cereal::JSONOutputArchive output(out);
    output(chain);
  }
  markov::Chain serialized;
  std::ifstream in_arc("output.txt");
  serialized.SerializeFrom<cereal::JSONInputArchive>(in_arc);
  {
    std::ofstream out("output2.txt");
    cereal::JSONOutputArchive output(out);
    output(serialized);
  }
}
