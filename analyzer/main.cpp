#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

#include <cereal/archives/json.hpp>

#include <tokens_storage/tokens_order_storage.hpp>

int main(int argc, char** argv) {
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
  if (argc < 3) {
    std::cout << "Usage is `analyzer list_of_files_to_tokenize token_file`\n";
    return 1;
  }
  markov::TokensOrderStorage tokens;
  std::ifstream list_to_tokenize(argv[1]);
  std::string fname_to_tokenize;
  while(std::getline(list_to_tokenize, fname_to_tokenize)) {
    std::ifstream file_to_tokenize(fname_to_tokenize);
    tokens.TokenizeFile(file_to_tokenize);
  }

  std::ofstream out(argv[2]);
  cereal::JSONOutputArchive out_archive(out);
  out_archive(tokens);

  /* std::ifstream in("financier.txt");
  markov::TokensOrderStorage tokens;
  tokens.TokenizeFile(in);
  std::deque<std::string> words = {"he", "had"};
  std::vector<std::string> sentence(words.begin(), words.end());
  for (int i = 0; i < 8; ++i) {
    auto word = tokens.GenerateNext(words.begin(), words.end());
    if (word.empty() || word == ".")
      break;
    words.pop_front();
    words.push_back(word);
    sentence.push_back(word);
  }
  for (auto& w : sentence) {
    std::cout << w << ' ';
  }*/
}
