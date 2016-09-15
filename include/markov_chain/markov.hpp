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

    template<class Archive>
    void save(Archive& archive) const {
      archive(*word, frequency);
    }

    template<class Archive>
    void load(Archive& archive, markov::WordWithFrequency& chain) {
      throw std::runtime_error("Not okay");
    }
  };

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

    template<class Archive>
    void save(Archive& archive) const {
      archive(freqs_);
    }

    template<class Archive>
    void load(Archive& archive) {
      // archive(inst.freqs_);
    }

   private: 
    FrequencyMap freqs_;
  };

  class Chain {
    friend class cereal::access;
   public:
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
    void save(Archive& archive) const {
      archive(head_);
    }

    template<class Archive>
    void load(Archive& archive) {
      // archive(inst.freqs_);
    }

   private:
    std::unordered_set<StrType> tokens_;
    WordsFrequency head_;
    int depth_;
  };
} // namespace markov


/*
template<class Archive>
inline void save(Archive& archive, const markov::Chain& inst) {
  archive(inst.head_);
}

template<class Archive>
inline void save(Archive& archive, const markov::WordsFrequency& inst) {
  // for (auto& word : inst.freqs_) {
  archive(inst.freqs_);
  //}
}

template<class Archive>
inline void save(Archive& archive, const markov::WordWithFrequency& chain) {
  archive(*chain.word, chain.frequency);
}

template<class Archive>
inline void load(Archive& archive, markov::Chain& inst) {
  //archive(inst.head_);
}

template<class Archive>
inline void load(Archive& archive, markov::WordsFrequency& inst) {
  // for (auto& word : inst.freqs_) {
  //archive(inst.freqs_);
  //}
}

template<class Archive>
inline void load(Archive& archive, markov::WordWithFrequency& chain) {
  //archive(*chain.word, chain.frequency);
}*/

#endif // MARKOV_HPP_
