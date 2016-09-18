#ifndef MARKOV_HPP_
#define MARKOV_HPP_

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <deque>
#include <istream>
#include <ostream>

#include "cereal/types/unordered_map.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/string.hpp"

namespace markov {
  using StrType = std::string;

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
  void load(Archive& archive, WordWithFrequency& chain) {
    std::string word;
    archive(cereal::make_nvp("word", word));
    archive(cereal::make_nvp("frequency", chain.frequency));
    auto in_tokens = static_cast<ArchiveAndTokens<Archive>&>(archive).tokens.insert(word);
    chain.word = &(*in_tokens.first);
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
    // Iterator is iterator to const StrType*
    template<class Iterator>
    void Load(Iterator begin, Iterator end) {
      FrequencyMap::iterator insert_result = freqs_.insert(
          std::make_pair<WordWithFrequency, std::unique_ptr<WordsFrequency>>(
            {*begin, 0}, nullptr)).first;
      ++insert_result->first.frequency;
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

   private: 
    /*template<class Archive>
    void save(Archive& archive) const {
      archive(cereal::make_nvp("map", freqs_));
    }

    template<class Archive>
    void load(Archive& archive) {
      throw std::runtime_error("Not okay");
    }*/

    template<class Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("map", freqs_));
    }

    FrequencyMap freqs_;
  };

  template <class Archive>
  struct ArchiveAndTokens : public Archive {
    ArchiveAndTokens(std::istream& stream, 
                     std::unordered_set<StrType>& in_tokens) : Archive(stream), tokens(in_tokens) {}
    std::unordered_set<StrType>& tokens;
  };

  class Chain {
   public:
    Chain() {}

    Chain(std::istream& in, int depth = 3)
      : depth_(depth)
    {
      TokenizeFile(in);
    }

    void TokenizeFile(std::istream& in) {
      std::string token;
      std::deque<const StrType*> last;
      bool process = false;
      while (in >> token || !last.empty()) {
        if (process) {
          head_.Load(last.begin(), last.end());
          last.pop_front();
        }
        if (in) {
          const StrType* token_ptr = &(*tokens_.insert(token).first);
          last.push_back(token_ptr);
          process = (last.size() == depth_);
        }
      }
    }

    void Show(std::ostream& out) {
      head_.Show(out, 0);
    }

    template<class Archive>
    void SerializeFrom(std::istream& stream) {
      ArchiveAndTokens<Archive> ar(stream, tokens_);
      ar(*this);
    }

   private:
    friend class cereal::access;

    /*template<class Archive>
    void save(Archive& archive) const {
      archive(cereal::make_nvp("head", head_));
    }

    template<class Archive>
    void load(Archive& archive) {
      throw std::runtime_error("Not okay");
    }*/

    template<class Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("depth", depth_));
      archive(cereal::make_nvp("head", head_));
    }

    std::unordered_set<StrType> tokens_;
    WordsFrequency head_;
    int depth_;
  };
} // namespace markov
/*
namespace cereal {
  //! Saving std::unique_ptr for non polymorphic types
  template <class Archive, class Deleter> inline
  void 
  CEREAL_SAVE_FUNCTION_NAME(Archive & ar, const std::unique_ptr<markov::WordsFrequency, Deleter>& ptr) {
    if (ptr)
      ar(*ptr);
  }

  //! Loading std::unique_ptr for non polymorphic types
  template <class Archive, class Deleter> inline
  void
  CEREAL_LOAD_FUNCTION_NAME(Archive & ar, std::unique_ptr<markov::WordsFrequency, Deleter>& ptr) {
    try {
      markov::WordsFrequency freqs;
      ar(cereal::make_nvp("map", freqs));
      ptr.reset(new markov::WordsFrequency(freqs));
    } catch (cereal::Exception& exc) {
    }
  }
}
*/

#endif // MARKOV_HPP_
