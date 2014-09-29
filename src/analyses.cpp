#include "analyses.h"

#include <fstream>
#include <mutex>

namespace CorpusAnalyser
{

AnalysisResults::AnalysisResults()
    :   selected(false),
        doInitial(kDoInitial),
        filename(),
        x_axis(),
        y_axis(),
        full(),
        initial() {}

std::once_flag flag;
const AnalysesMap& GetAnalysesMap()
{
    static AnalysesMap map;
    std::call_once(flag,
                   []()
                   {
                       map.insert(std::make_pair(Analyses::kXvsX, "X vs X"));
                       map.insert(std::make_pair(Analyses::kXvsY, "X vs Y"));
                       map.insert(std::make_pair(Analyses::kYvsX, "Y vs X"));
                       map.insert(std::make_pair(Analyses::kYvsY, "Y vs Y"));
                   });
    return map;
}

void SetNewAnalysis(const Analyses& analysisType,
                    const AxisX& xAxis,
                    const AxisY& yAxis,
                    const std::string& name,
                    const bool& doInitial,
                    SelectedAnalyses& selected)
{
    auto& res = selected[CorpusAnalyser::ToUnderlying(analysisType)];
    res.selected = true;
    res.filename = name;
    res.x_axis = xAxis;
    res.y_axis = yAxis;
    res.doInitial = doInitial;
    res.full = std::make_shared<std::ofstream>(name + CorpusAnalyser::kResultExtension);
    if (doInitial)
        res.initial = std::make_shared<std::ofstream>(name + "_init" + CorpusAnalyser::kResultExtension);
}

}  // namespace CorpusAnalyser

