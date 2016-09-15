#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <deque>
#include <istream>
#include <ostream>

namespace markov {
  using StrType = std::string;

  struct WordWithFrequency {
    const StrType* word;
    // WordWithFrequency is used in unorder_map
    mutable int frequency;
  };

  bool operator==(const WordWithFrequency& lhs, const WordWithFrequency& rhs) {
    return *lhs.word == *rhs.word;
  }
}

namespace std {
  template <> 
  struct hash<markov::WordWithFrequency> {
    size_t operator()(const markov::WordWithFrequency& x) const {
      return hash<markov::StrType>()(*(x.word));
    }
  };
}

namespace markov {
  class WordsFrequency {
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
    FrequencyMap freqs_;
  };

  class Chain {
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

   private:
    std::unordered_set<StrType> tokens_;
    WordsFrequency head_;
    int depth_;
  };
}
