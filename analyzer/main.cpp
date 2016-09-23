#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

#include <cereal/archives/json.hpp>

#include <tokens_storage/tokens_order_storage.hpp>

int main() {
  /*{
    std::ifstream in("input.txt");
    std::ofstream out("output.txt");
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
  }*/
  std::ifstream in("financier.txt");
  auto tokens = markov::TokensOrderStorage::TokenizeFile(in);
  std::deque<std::string> words = {"he", "had"};
  std::vector<std::string> sentence(words.begin(), words.end());
  for (int i = 0; i < 8; ++i) {
    auto word = tokens->GenerateNext(words.begin(), words.end());
    if (word.empty() || word == ".")
      break;
    words.pop_front();
    words.push_back(word);
    sentence.push_back(word);
  }
  for (auto& w : sentence) {
    std::cout << w << ' ';
  }
}
