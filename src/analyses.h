#ifndef ANALYSES_H
#define ANALYSES_H

#include <map>
#include <memory>
#include <vector>

#include "types.h"

namespace CorpusAnalyser
{

struct AnalysisResults
{
    AnalysisResults();
    bool selected, doInitial;
    std::string filename;
    AxisX x_axis;
    AxisY y_axis;
    std::shared_ptr<std::ofstream> full, initial;
};
typedef std::vector<AnalysisResults> SelectedAnalyses;

enum class Analyses : int
{
    kXvsX = 0,
    kXvsY,
    kYvsX,
    kYvsY
};
typedef std::map<Analyses, FilterName> AnalysesMap;

const AnalysesMap& GetAnalysesMap();

void SetNewAnalysis(const Analyses& analysisType,
                    const AxisX& xAxis,
                    const AxisY& yAxis,
                    const std::string& name,
                    const bool& doInitial,
                    SelectedAnalyses& selected);

}  // namespace CorpusAnalyser

#endif // ANALYSES_H
