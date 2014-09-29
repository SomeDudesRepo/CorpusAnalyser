#include "corpusanalysis.h"

#include <fstream>

#include <QFile>
#include <QFileInfo>

#include "analyses.h"
#include "log.h"
#include "runner.h"

namespace CorpusAnalyser
{

namespace
{

int CountOcurrences(const Word& w,
                    const Pattern& pattern,
                    const bool& just_initial)
{
//    Log("CountOcurrences " + pattern);
    if (just_initial)
        return w.find(pattern) == 0U ? 1 : 0;

    int occurrences(0);
    Word::size_type start(0);

    while ((start = w.find(pattern, start)) != Word::npos)
    {
        ++occurrences;
        start += pattern.length();  // see the note
    }
   return occurrences;
}

void AddHeaders(SelectedAnalyses& selected)
{
    for (const auto& element : selected)
    {
        if (!element.selected)
            continue;

        (*element.full) << " ";
        if (element.doInitial)
            (*element.initial) << " ";
        for (const char& y : element.y_axis)
        {
            (*element.full) << "," << y;
            if (element.doInitial)
                (*element.initial) << "," << y;
        }
        (*element.full) << std::endl;
        if (element.doInitial)
            (*element.initial) << std::endl;
    }
}

void AnalysePatternVsWords(const Words words,
                           const Vowels& vowels,
                           Pattern& pattern,
                           SelectedAnalyses::value_type& element)
{
    Log("AnalyseOneFilter " + pattern);
    int count(0), init_count(0);
    for (const char& v : vowels)
    {
        pattern[1] = v;
        for (const auto& word : words)
        {
            count += CountOcurrences(word, pattern, false);
            if (element.doInitial)
                init_count += CountOcurrences(word, pattern, true);
        }
    }
    (*element.full) << "," << count;
    if (element.doInitial)
        (*element.initial) << "," << init_count;
}

void AnalyseOneFilter(const Words& words,
                      const Vowels& vowels,
                      SelectedAnalyses::value_type& element)
{
    Log("AnalyseOneFilter " + element.filename);
    for (const char& x : element.x_axis)
    {
        (*element.full) << x;
        if (element.doInitial)
            (*element.initial) << x;
        for (const char& y : element.y_axis)
        {
            Pattern pattern(1, x);
            pattern += "_";
            pattern += y;
            AnalysePatternVsWords(words, vowels, pattern, element);
        }
        (*element.full) << std::endl;
        if (element.doInitial)
            (*element.initial) << std::endl;
    }
}

std::string GetFileRoot(const std::string& fullPath)
{
    const QFile file(QString::fromStdString(fullPath));
    const QFileInfo info(file);
    const QString fileRoot(info.fileName());
    return fileRoot.toStdString();
}

}  // namespace

void RunAnalysis(const Words& words,
                 const Vowels& vowels,
                 SelectedAnalyses& selected,
                 Runner& runner)
{
    AddHeaders(selected);

    for (auto& element : selected)
    {
        if (!element.selected)
            continue;
        AnalyseOneFilter(words, vowels, element);
        Log("Done " + element.filename);
        runner.Update(1, QString::fromStdString("Done " + GetFileRoot(element.filename)));
    }
}

}  // namespace CorpusAnalyser
