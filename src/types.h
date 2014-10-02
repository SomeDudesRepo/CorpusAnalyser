#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

namespace CorpusAnalyser
{

// types
typedef std::string Word;
typedef std::vector<Word> Words;
typedef std::string Vowels;
typedef std::string Pattern;
typedef std::string LogMessage;
typedef std::string DateAndTime;
typedef std::string AxisX;
typedef std::string AxisY;
typedef std::string FilterName;

// constants
const std::string kVersion("0.2.2");
const std::string kResultExtension(".csv");
const std::string kInitSuffix("_init");
const bool kDoInitial(true);

// converters
template <typename E>
typename std::underlying_type<E>::type ToUnderlying(E e)
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

}  // namespace CorpusAnalyser

#endif // TYPES_H
