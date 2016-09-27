#ifndef TOKENS_ORDER_STORAGE_IMPL_HPP_
#define TOKENS_ORDER_STORAGE_IMPL_HPP_

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <deque>
#include <istream>
#include <ostream>
#include <random>
#include <chrono>

#include "cereal/types/unordered_map.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/string.hpp"

namespace markov {
  template <class Archive>
  struct ArchiveAndTokens : public Archive {
    ArchiveAndTokens(std::istream& stream,
                     std::unordered_set<StrType>& in_tokens) : Archive(stream), tokens(in_tokens) {}
    std::unordered_set<StrType>& tokens;
  };

  struct WordWithFrequency {
    const StrType* word;
    // WordWithFrequency is used in unorder_map
    mutable int frequency;
  };

  template<class Archive>
  void save(Archive& archive, const WordWithFrequency& inst) {
    archive(cereal::make_nvp("word", *inst.word));
    archive(cereal::make_nvp("frequency", inst.frequency));
  }

  template<class Archive>
  void load(Archive& archive, WordWithFrequency& TokensOrderStorage) {
    std::string word;
    archive(cereal::make_nvp("word", word));
    archive(cereal::make_nvp("frequency", TokensOrderStorage.frequency));
    auto in_tokens = static_cast<ArchiveAndTokens<Archive>&>(archive).tokens.insert(word);
    TokensOrderStorage.word = &(*in_tokens.first);
  }

  bool operator==(const WordWithFrequency& lhs, const WordWithFrequency& rhs) {
    return *lhs.word == *rhs.word;
  }
} //namespace markov

namespace std {
  template <>
  struct hash<markov::WordWithFrequency> {
    size_t operator()(const markov::WordWithFrequency& x) const {
      return hash<markov::StrType>()(*(x.word));
    }
  };
} //namespace std

namespace markov {
  class WordsFrequency {
    friend class cereal::access;
   using FrequencyMap = std::unordered_map<WordWithFrequency, std::unique_ptr<WordsFrequency>>;
   public:
    WordsFrequency() : freq_sum_(0) {}

    // Iterator to const StrType*
    template<class Iterator>
    void Load(Iterator begin, Iterator end) {
      FrequencyMap::iterator insert_result = freqs_.insert(
          std::make_pair<WordWithFrequency, std::unique_ptr<WordsFrequency>>(
            {*begin, 0}, nullptr)).first;
      ++insert_result->first.frequency;
      ++freq_sum_;
      ++begin;
      if (begin == end) {
        return;
      } else {
        if (!insert_result->second) {
          insert_result->second.reset(new WordsFrequency);
        }
        insert_result->second->Load(begin, end);
      }
    }

    void Show(std::ostream& out, int offset_size) {
      std::string offset(offset_size, ' ');
      for (auto it = freqs_.begin(); it != freqs_.end();) {
        auto& f = *it;
        ++it;
        out << *(f.first.word) << ':' << f.first.frequency << ' ';
        if (f.second) {
          f.second->Show(out, offset_size + f.first.word->size() + 2);
        } else {
          out << '\n';
          if (it != freqs_.end())
            out << offset;
        }
      }
    }

    const FrequencyMap& frequencies() const { return freqs_; }
    int freq_sum() const { return freq_sum_; }
   private:
    template<class Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("map", freqs_));
      for (auto& f : freqs_) {
        freq_sum_ += f.first.frequency;
      }
    }

    FrequencyMap freqs_;
    int freq_sum_;
  };


  inline TokensOrderStorage::TokensOrderStorage(int depth)
    : head_(new WordsFrequency)
    , depth_(depth) {}

  inline
  void
  TokensOrderStorage::TokenizeFile(std::istream& in) {
    std::deque<StrType> tokens;
    std::deque<const StrType*> last;
    bool continue_parse = true;
    while (continue_parse) {
      continue_parse = false;
      if (in) { // read token
        continue_parse = true;
        std::string token;
        in >> token;
        // split token
        const std::string delimeters = ",.";
        int start = 0;
        int len = 0;
        for (auto c : token) {
          if (delimeters.find(c) != std::string::npos) {
            if (len > 0) { // if something before delimeter - extract it
              tokens.push_back(token.substr(start, len));
            }
            tokens.push_back(token.substr(start + len, 1));
            start += len + 1;
            len = 0;
          } else {
            ++len;
          }
        }
        if (len > 0)
          tokens.push_back(token.substr(start, len));
      }
      while(tokens.size() > 0) {
        const StrType* token_ptr = &(*tokens_.insert(tokens.front()).first);
        tokens.pop_front();
        last.push_back(token_ptr);
        if (last.size() >= depth_) {
          head_->Load(last.begin(), last.end());
          last.pop_front();
        }
      }
    }
  }

  template<class Archive>
  inline
  std::unique_ptr<TokensOrderStorage>
  TokensOrderStorage::SerializeFrom(std::istream& stream) {
    std::unique_ptr<TokensOrderStorage> storage(new TokensOrderStorage(0));
    ArchiveAndTokens<Archive> ar(stream, storage->tokens_);
    ar(*storage);
    return storage;
  }

  inline void TokensOrderStorage::Show(std::ostream& out) {
    head_->Show(out, 0);
  }

  // Iterator to StrType
  template<class Iterator>
  inline StrType TokensOrderStorage::GenerateNext(Iterator begin, Iterator end) {
    StrType word;
    const WordsFrequency* current_map = nullptr;
    bool found = false;
    while(begin != end && !found) {
      auto start_from = begin;
      current_map = head_.get();
      while (start_from != end) {
        auto found_in_tokens = tokens_.find(*start_from);
        if (found_in_tokens != tokens_.end()) {
          WordWithFrequency wwf;
          wwf.word = &(*found_in_tokens);
          auto found_path = current_map->frequencies().find(wwf);
          if (found_path != current_map->frequencies().end() && found_path->second) {
            ++start_from;
            word = *(found_path->first.word);
            current_map = found_path->second.get();
            continue;
          }
        }
        ++begin;
        start_from = end;
        current_map = nullptr;
      }
      found = true;
    }
    if (current_map != nullptr) {
      if (current_map->frequencies().size() == 1) {
        word = *(current_map->frequencies().begin()->first.word);
      } else {
        // choose random word
        std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
        int guess = std::uniform_int_distribution<int> (0, current_map->freq_sum() - 1)(gen);
        int prev_sum = 0;
        for (const auto& next_word : current_map->frequencies()) {
          int cur_sum = prev_sum + next_word.first.frequency;
          if (guess < cur_sum && guess >= prev_sum) {
            word = *(next_word.first.word);
            break;
          }
          prev_sum = cur_sum;
        }
      }
    }
    return word;
  }


  template<class Archive>
  inline void TokensOrderStorage::serialize(Archive& archive) {
    archive(cereal::make_nvp("depth", depth_));
    archive(cereal::make_nvp("head", *(head_)));
  }
} // namespace markov

#endif // TOKENS_ORDER_STORAGE_IMPL_HPP_
