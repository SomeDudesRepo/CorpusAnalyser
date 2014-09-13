#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <QDir>

namespace
{

const std::string kExtension(".csv");

typedef std::wstring Vowels;
typedef std::wstring AxisX;
typedef std::wstring AxisY;

typedef std::wstring Word;
typedef std::vector<Word> Words;

struct AnalysisResults
{
    AnalysisResults()
        :   selected(false),
            full_count(-1),
            initial_count(-1),
            filename(),
            full(),
            initial() {}
    bool selected;
    int full_count, initial_count;
    std::string filename;
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
    std::wstring temp;
    char ch('\0');
    std::vector<std::wstring> words;
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
                    const std::wstring& pattern,
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

void AddHeaders(const AxisY& y_axis,
                SelectedAnalyses& selected)
{
    for (const auto& element : selected)
    {
        if (!element.selected)
            continue;

        (*element.full) << " ";
        (*element.initial) << " ";
        for (const char& y : y_axis)
        {
            (*element.full) << "," << y;
            (*element.initial) << "," << y;
        }
        (*element.full) << std::endl;
        (*element.initial) << std::endl;
    }
}

void RunAnalysis(const Words& words,
                 const AxisX& x_axis,
                 const AxisY& y_axis,
                 const Vowels& vowels,
                 SelectedAnalyses& selected)
{
    AddHeaders(y_axis, selected);

/*    for (const char& x : x_axis)
    {
        full << x;
        initial << x;
        for (const char& y : y_axis)
        {
            int count(0), init_count(0);
            std::wstring pattern(1, x);
            pattern += L"_";
            pattern += y;
            for (const char& v : vowels)
            {
                pattern[1] = v;
                for (const Word& w : words)
                {
                    count += CountOcurrences(w, pattern, false);
                    init_count += CountOcurrences(w, pattern, true);
                }
            }
            full << "," << count;
            initial << "," << init_count;
        }
        full << std::endl;
        initial << std::endl;
    }*/
}

void SetNewAnalysis(const Analyses& analysisType,
                    const std::string& name,
                    SelectedAnalyses& selected)
{
    auto& res(selected[to_underlying(analysisType)]);
    res.selected = true;
    res.filename = name;
    res.full = std::make_shared<std::ofstream>(name);
    res.initial = std::make_shared<std::ofstream>(name + "_init");
}

SelectedAnalyses CheckInputs(Ui::MainWindow& ui)
{
    SelectedAnalyses selected(4, AnalysisResults());
    bool b(false);
    if (ui.chk_x_vs_x_->isChecked())
    {
        SetNewAnalysis(Analyses::kXvsX, "x_vs_x", selected);
        b = true;
    }
    if (ui.chk_x_vs_y_->isChecked())
    {
        SetNewAnalysis(Analyses::kXvsY, "x_vs_y", selected);
        b = true;
    }
    if (ui.chk_y_vs_x_->isChecked())
    {
        SetNewAnalysis(Analyses::kYvsX, "y_vs_x", selected);
        b = true;
    }
    if (ui.chk_y_vs_y_->isChecked())
    {
        SetNewAnalysis(Analyses::kYvsY, "y_vs_y", selected);
        b = true;
    }
    if (b)
        throw "Select at least one ";

    return selected;
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    :   QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    on_edt_corpus__editingFinished();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_run__clicked()
{
    try
    {
        this->statusBar()->showMessage("");
        SelectedAnalyses selected(CheckInputs(*ui));

        // Get strings from file
        Words words(GetWordsFromFile(ui->edt_corpus_->text()));
        const std::string directory(ui->edt_corpus_->text().toStdString());
        std::string message("Words found: " + std::to_string(words.size()) +
                            " at " + directory);
        this->statusBar()->showMessage(QString::fromStdString(message));

        // Run analyses
        const std::wstring x_axis(ui->edt_x_axis_->text().toStdWString()),
                           y_axis(ui->edt_y_axis_->text().toStdWString()),
                           vowels(ui->edt_vowels_->text().toStdWString());
        RunAnalysis(words, x_axis, x_axis, vowels, selected);
//        message = "Done with x vs x.";
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));
//        this->statusBar()->showMessage(QString::fromStdString(message));
//        RunAnalysis(words, x_axis, y_axis, vowels, "x_vs_y", directory);
//        message = "Done with x vs y.";
//        this->statusBar()->showMessage(QString::fromStdString(message));
//        RunAnalysis(words, y_axis, x_axis, vowels, "y_vs_x", directory);
//        message = "Done with y vs x.";
//        this->statusBar()->showMessage(QString::fromStdString(message));
//        RunAnalysis(words, y_axis, y_axis, vowels, "y_vs_y", directory);
//        message = "Done with y vs y.";
//        this->statusBar()->showMessage(QString::fromStdString(message));
    }
    catch(...)
    {
        this->statusBar()->showMessage("Failed!");
    }
}

void MainWindow::on_edt_corpus__editingFinished()
{
    QDir dir(ui->edt_corpus_->text());
    dir.cdUp();
    const std::string directory(dir.path().toStdString() + "/Results/");
    ui->edt_output_dir_->setText(QString::fromStdString(directory));
}
