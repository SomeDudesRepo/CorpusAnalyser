#ifndef WORDEXTRACTOR_H
#define WORDEXTRACTOR_H

#include <QString>

#include "types.h"

namespace CorpusAnalyser
{

Words GetWordsFromFile(const QString& path);

}  // namespace CorpusAnalyser

#endif  // WORDEXTRACTOR_H
