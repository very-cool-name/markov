#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

#include <cereal/archives/json.hpp>

#include <tokens_storage/tokens_order_storage.hpp>

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cout << "Usage is `analyzer tokens_file word1 word2`\n";
    return 1;
  }
  std::ifstream in(argv[1]);
  auto chain = markov::TokensOrderStorage::SerializeFrom<cereal::JSONInputArchive>(in);
  std::deque<std::string> words = {argv[2], argv[3]};
  std::vector<std::string> sentence(words.begin(), words.end());
  for (int i = 0; i < 8; ++i) {
    auto word = chain->GenerateNext(words.begin(), words.end());
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
