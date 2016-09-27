#ifndef TOKENS_ORDER_STORAGE_HPP_
#define TOKENS_ORDER_STORAGE_HPP_

#include <unordered_set>
#include <memory>
#include <string>
#include <iosfwd>

namespace markov {
  using StrType = std::string;

  class TokensOrderStorage {
   public:

    explicit TokensOrderStorage(int depth = 3);
    
    template<class Archive>
    static std::unique_ptr<TokensOrderStorage> SerializeFrom(std::istream& stream);

    void TokenizeFile(std::istream& in);
    void Show(std::ostream& out);

    // Iterator to StrType
    template<class Iterator>
    StrType GenerateNext(Iterator begin, Iterator end);

   private:
    friend class cereal::access;


    template<class Archive>
    void serialize(Archive& archive);

    std::unordered_set<StrType> tokens_;
    std::unique_ptr<class WordsFrequency> head_;
    int depth_;
  };
} // namespace markov

#include "tokens_order_storage_impl.hpp"

#endif // TOKENS_ORDER_STORAGE_HPP_
