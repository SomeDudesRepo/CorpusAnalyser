#ifndef CORPUSANALYSIS_H
#define CORPUSANALYSIS_H

#include "analyses.h"

namespace CorpusAnalyser
{

class Runner;

void RunAnalysis(const Words& words,
                 const Vowels& vowels,
                 SelectedAnalyses& selected,
                 Runner& runner);

}  // namespace CorpusAnalyser

#endif  // CORPUSANALYSIS_H
