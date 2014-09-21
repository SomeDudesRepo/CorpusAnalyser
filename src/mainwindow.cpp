#include "mainwindow.h"
#include "runner.h"
#include "ui_mainwindow.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <future>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

namespace
{

typedef std::string DateAndTime;
DateAndTime NowDateAndTime()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto fractional_seconds = ms.count() % 1000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X.") << fractional_seconds;
    return DateAndTime(ss.str());
}

typedef std::string LogMessage;
void Log(const LogMessage& msg)
{
    static std::ofstream log("C:\\Users\\Dan\\Documents\\GitHub\\CorpusAnalyser\\test_files\\log.txt");
    static std::mutex mutex;

    {
        std::lock_guard<std::mutex> loch(mutex);
        const auto now = NowDateAndTime();
        log << std::this_thread::get_id() << " - " << now << ": " << msg << std::endl;
    }
}

const std::string kExtension(".csv");

typedef std::string Vowels;
typedef std::string AxisX;
typedef std::string AxisY;

typedef std::string Pattern;
typedef std::string Word;
typedef std::vector<Word> Words;

typedef std::string FilterName;

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
typedef std::map<Analyses, FilterName> AnalysesMap;

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
    Log("AnalyseOneFilter " + pattern);
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
    Log("AnalyseOneFilter " + element.filename);
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
        Log("Done " + element.filename);
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
    auto selection = ui.mLstFilters->selectedItems();
    if (selection.empty())
        throw "Select at least one filter!";

    for (const auto& item : selection)
    {
        switch (item->type())
        {
            case Analyses::kXvsX:
                SetNewAnalysis(Analyses::kXvsX, x_axis, x_axis, outDir + "/x_vs_x", selected);
                break;
            case Analyses::kXvsY:
                SetNewAnalysis(Analyses::kXvsY, x_axis, y_axis, outDir + "/x_vs_y", selected);
                break;
            case Analyses::kYvsX:
                SetNewAnalysis(Analyses::kYvsX, y_axis, x_axis, outDir + "/y_vs_x", selected);
                break;
            case Analyses::kYvsY:
                SetNewAnalysis(Analyses::kYvsY, y_axis, y_axis, outDir + "/y_vs_y", selected);
                break;
        }
    }
    return selected;
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    :   QMainWindow(parent),
        ui(new Ui::MainWindow),
        mQMovie(new QMovie(":/images/ajax-loader.gif"))
{
    ui->setupUi(this);
    this->statusBar()->showMessage("Select at least one analysis");
    const AnalysesMap& map(GetAnalysesMap());
    for (const auto& item : map)
    {
        ui->mLstFilters->addItem(
            new QListWidgetItem(QString::fromStdString(item.second),
                                ui->mLstFilters,
                                to_underlying(item.first)));
    }
    setWindowFlags(Qt::Window |
                   Qt::WindowTitleHint |
                   Qt::CustomizeWindowHint |
                   Qt::WindowMinimizeButtonHint |
                   Qt::WindowCloseButtonHint);
    ui->mLblGif->setMovie(mQMovie.get());
    ui->mLblGif->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
    mQMovie->stop();
}

void MainWindow::on_mBtnRun_clicked()
{
    try
    {
        ui->mBtnRun->setEnabled(false);
        ui->mLblGif->show();
        mQMovie->start();
        ui->mLblGif->repaint();

        auto func = std::bind(&MainWindow::UpdateMovie, this);
        std::async(std::launch::async,
                   [this, func] ()
                   {
                       try
                       {
                           auto selected(CheckInputs(*ui));
                           // Get strings from file
                           Words words(GetWordsFromFile(ui->mEdtCorpus->text()));

                           // Run analyses
                           const Vowels vowels(ui->mEdtVowels->text().toStdString());
                           std::unique_ptr<Runner> runner(new Runner);
                           connect(runner.get(), SIGNAL(done()), this, SLOT(UpdateMovie()));
                           RunAnalysis(words, vowels, selected);
                           runner->Completed();
                           std::this_thread::sleep_for(std::chrono::seconds(2));
                           Log("Analysis done!");
                       }
                       catch(...) { Log("Failure in analysis"); }
                   });
    }
    catch(...)
    {
        QMessageBox::critical(this, tr("Failure"), tr("There has been some error!"));
    }
}

void  MainWindow::UpdateMovie()
{
    Log("UpdateMovie");
    mQMovie->stop();
    ui->mLblGif->hide();
    ui->mLblGif->repaint();
    this->statusBar()->showMessage(QString::fromStdString("Finished!"));
}

void MainWindow::on_mBtnCorpus_clicked()
{
    auto corpus = QFileDialog::getOpenFileName(
                      this,
                      tr("Select a corpus..."),
                      tr("C:\\Users\\Dan\\Documents\\GitHub\\CorpusAnalyser\\test_files\\"));
    ui->mEdtCorpus->setText(corpus);
}

void MainWindow::on_mBtnOutputDir_clicked()
{
    auto outputDir = QFileDialog::getExistingDirectory(
                         this,
                         tr("Select an output directory..."),
                         tr("C:\\Users\\Dan\\Documents\\GitHub\\CorpusAnalyser\\test_files\\"));
    ui->mEdtOutputDir->setText(outputDir);
}

void MainWindow::on_action_Exit_triggered()
{
    QApplication::quit();
}
