#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

namespace
{

const std::string kExtension(".csv");

typedef std::string Vowels;
typedef std::string AxisX;
typedef std::string AxisY;

typedef std::string Pattern;
typedef std::string Word;
typedef std::vector<Word> Words;

struct AnalysisResults
{
    AnalysisResults()
        :   selected(false),
            filename(),
            x_axis(),
            y_axis(),
            full(),
            initial() {}
    bool selected;
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

template <typename E>
typename std::underlying_type<E>::type to_underlying(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

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

int CountOcurrences(const Word& w,
                    const Pattern& pattern,
                    const bool& just_initial)
{
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
        (*element.initial) << " ";
        for (const char& y : element.y_axis)
        {
            (*element.full) << "," << y;
            (*element.initial) << "," << y;
        }
        (*element.full) << std::endl;
        (*element.initial) << std::endl;
    }
}

void AnalysePatternVsWords(const Words words,
                           const Vowels& vowels,
                           Pattern& pattern,
                           SelectedAnalyses::value_type& element)
{
    int count(0), init_count(0);
    for (const char& v : vowels)
    {
        pattern[1] = v;
        for (const auto& word : words)
        {
            count += CountOcurrences(word, pattern, false);
            init_count += CountOcurrences(word, pattern, true);
        }
    }
    (*element.full) << "," << count;
    (*element.initial) << "," << init_count;
}

void AnalyseOneFilter(const Words& words,
                      const Vowels& vowels,
                      SelectedAnalyses::value_type& element)
{
    for (const char& x : element.x_axis)
    {
        (*element.full) << x;
        (*element.initial) << x;
        for (const char& y : element.y_axis)
        {
            Pattern pattern(1, x);
            pattern += "_";
            pattern += y;
            AnalysePatternVsWords(words, vowels, pattern, element);
        }
        (*element.full) << std::endl;
        (*element.initial) << std::endl;
    }
}

void RunAnalysis(const Words& words,
                 const Vowels& vowels,
                 SelectedAnalyses& selected)
{
    AddHeaders(selected);

    for (auto& element : selected)
    {
        if (!element.selected)
            continue;
        AnalyseOneFilter(words, vowels, element);
    }
}

void SetNewAnalysis(const Analyses& analysisType,
                    const AxisX& xAxis,
                    const AxisY& yAxis,
                    const std::string& name,
                    SelectedAnalyses& selected)
{
    auto& res = selected[to_underlying(analysisType)];
    res.selected = true;
    res.filename = name;
    res.x_axis = xAxis;
    res.y_axis = yAxis;
    res.full = std::make_shared<std::ofstream>(name + kExtension);
    res.initial = std::make_shared<std::ofstream>(name + "_init" + kExtension);
}

void CheckAxesAndVowels(Ui::MainWindow& ui)
{
    if (ui.mEdtAxisX->text().isEmpty() ||
        ui.mEdtAxisY->text().isEmpty() ||
        ui.mEdtVowels->text().isEmpty() ||
        ui.mEdtCorpus->text().isEmpty() ||
        ui.mEdtOutputDir->text().isEmpty())
        throw "Empty fields!";
}

SelectedAnalyses CheckInputs(Ui::MainWindow& ui)
{
    CheckAxesAndVowels(ui);
    const AxisX x_axis(ui.mEdtAxisX->text().toStdString());
    const AxisY y_axis(ui.mEdtAxisY->text().toStdString());
    const std::string outDir(ui.mEdtOutputDir->text().toStdString());
    SelectedAnalyses selected(4, AnalysisResults());
    bool b(false);
    if (ui.mChkXvsX->isChecked())
    {
        SetNewAnalysis(Analyses::kXvsX, x_axis, x_axis, outDir + "/x_vs_x", selected);
        b = true;
    }
    if (ui.mChkXvsY->isChecked())
    {
        SetNewAnalysis(Analyses::kXvsY, x_axis, y_axis, outDir + "/x_vs_y", selected);
        b = true;
    }
    if (ui.mChkYvsX->isChecked())
    {
        SetNewAnalysis(Analyses::kYvsX, y_axis, x_axis, outDir + "/y_vs_x", selected);
        b = true;
    }
    if (ui.mChkYvsY->isChecked())
    {
        SetNewAnalysis(Analyses::kYvsY, y_axis, y_axis, outDir + "/y_vs_y", selected);
        b = true;
    }
    if (!b)
        throw "Select at least one ";

    return selected;
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    :   QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->statusBar()->showMessage("Select at least one analysis");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_mBtnRun_clicked()
{
    try
    {
        SelectedAnalyses selected(CheckInputs(*ui));

        // Get strings from file
        Words words(GetWordsFromFile(ui->mEdtCorpus->text()));
        const std::string directory(ui->mEdtCorpus->text().toStdString());
        const std::string message("Words found: " + std::to_string(words.size()) +
                                  " at " + directory);
        this->statusBar()->showMessage(QString::fromStdString(message));

        // Run analyses
        const Vowels vowels(ui->mEdtVowels->text().toStdString());
        RunAnalysis(words, vowels, selected);
    }
    catch(...)
    {
        QMessageBox::critical(this, tr("Failure"), tr("There has been some error!"));
    }
}

void MainWindow::on_mBtnCorpus_clicked()
{
    auto corpus = QFileDialog::getOpenFileName(
                      this,
                      tr("Select a corpus..."),
                      tr("D:\\Learn\\Github\\CorpusAnalyser\\test_files"));
    ui->mEdtCorpus->setText(corpus);
}

void MainWindow::on_mBtnOutputDir_clicked()
{
    auto outputDir = QFileDialog::getExistingDirectory(
                         this,
                         tr("Select an output directory..."),
                         tr("D:\\Learn\\Github\\CorpusAnalyser\\test_files"));
    ui->mEdtOutputDir->setText(outputDir);
}
