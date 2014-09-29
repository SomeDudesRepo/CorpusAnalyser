#include "wordextractor.h"

#include <fstream>

namespace CorpusAnalyser
{

Words GetWordsFromFile(const QString& path)
{
    std::ifstream ifs(path.toStdString(), std::ios::binary);
    Word temp;
    char ch('\0');
    Words words;
    while (ifs >> std::noskipws >> ch)
    {
        if (ch == ' ' || ch == EOF)
        {
            words.push_back(temp);
            temp.clear();
        }
        else
        {
            temp += ch;
        }
    }
    words.push_back(temp);

    return words;
}

}  // namespace CorpusAnalyser

