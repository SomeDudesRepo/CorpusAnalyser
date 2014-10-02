#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <future>

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>

#include "corpusanalysis.h"
#include "log.h"
#include "runner.h"
#include "wordextractor.h"

namespace
{

void CheckAxesAndVowels(Ui::MainWindow& ui)
{
    if (ui.mEdtAxisX->text().isEmpty() ||
        ui.mEdtAxisY->text().isEmpty() ||
        ui.mEdtVowels->text().isEmpty() ||
        ui.mEdtCorpus->text().isEmpty() ||
        ui.mEdtOutputDir->text().isEmpty())
        throw std::string("Empty fields!");
}

CorpusAnalyser::SelectedAnalyses CheckInputs(Ui::MainWindow& ui)
{
    CheckAxesAndVowels(ui);
    const CorpusAnalyser::AxisX x_axis(ui.mEdtAxisX->text().toStdString());
    const CorpusAnalyser::AxisY y_axis(ui.mEdtAxisY->text().toStdString());
    const std::string outDir(ui.mEdtOutputDir->text().toStdString());
    CorpusAnalyser::SelectedAnalyses selected(4, CorpusAnalyser::AnalysisResults());
    auto selection = ui.mLstFilters->selectedItems();
    const auto doInitial = ui.mChkInitial->isChecked();
    if (selection.empty())
        throw std::string("Select at least one filter!");

    for (const auto& item : selection)
    {
        switch (item->type())
        {
            case CorpusAnalyser::Analyses::kXvsX:
                CorpusAnalyser::SetNewAnalysis(CorpusAnalyser::Analyses::kXvsX, x_axis, x_axis,
                                               outDir + "/x_vs_x", doInitial, selected);
                break;
            case CorpusAnalyser::Analyses::kXvsY:
                CorpusAnalyser::SetNewAnalysis(CorpusAnalyser::Analyses::kXvsY, x_axis, y_axis,
                                               outDir + "/x_vs_y", doInitial, selected);
                break;
            case CorpusAnalyser::Analyses::kYvsX:
                CorpusAnalyser::SetNewAnalysis(CorpusAnalyser::Analyses::kYvsX, y_axis, x_axis,
                                               outDir + "/y_vs_x", doInitial, selected);
                break;
            case CorpusAnalyser::Analyses::kYvsY:
                CorpusAnalyser::SetNewAnalysis(CorpusAnalyser::Analyses::kYvsY, y_axis, y_axis,
                                               outDir + "/y_vs_y", doInitial, selected);
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
    const CorpusAnalyser::AnalysesMap& map(CorpusAnalyser::GetAnalysesMap());
    for (const auto& item : map)
    {
        ui->mLstFilters->addItem(
            new QListWidgetItem(QString::fromStdString(item.second),
                                ui->mLstFilters,
                                CorpusAnalyser::ToUnderlying(item.first)));
    }
    setWindowFlags(Qt::Window |
                   Qt::WindowTitleHint |
                   Qt::CustomizeWindowHint |
                   Qt::WindowMinimizeButtonHint |
                   Qt::WindowCloseButtonHint);
    ui->mLblGif->setMovie(mQMovie.get());
    ui->mLblGif->hide();
    ui->mChkInitial->setChecked(CorpusAnalyser::kDoInitial);
    ui->mActResultFolder->setEnabled(!ui->mEdtOutputDir->text().isEmpty());
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_mBtnRun_clicked()
{
    std::async(std::launch::async,
               [this] ()
               {
                   std::unique_ptr<CorpusAnalyser::Runner> runner(new CorpusAnalyser::Runner);
                   connect(runner.get(), SIGNAL(update(const int&, const QString&)),
                           this, SLOT(UpdateUi(const int&, const QString&)));
                   runner->Update(0, "");
                   try
                   {
                       auto selected(CheckInputs(*ui));
                       runner->Update(1, QString::fromStdString("Checked inputs..."));

                       // Get strings from file
                       CorpusAnalyser::Words words(
                           CorpusAnalyser::GetWordsFromFile(ui->mEdtCorpus->text()));
                       runner->Update(1, QString::fromStdString("Words found: " +
                                                                std::to_string(words.size())));

                       // Run analyses
                       const CorpusAnalyser::Vowels vowels(ui->mEdtVowels->text().toStdString());
                       CorpusAnalyser::RunAnalysis(words, vowels, selected, *runner);
                       runner->Update(2, QString::fromStdString("Finished!"));
                   }
                   catch(const std::string& s)
                   {
                       runner->Update(3, QString::fromStdString(s));
                   }
                   catch(...)
                   {
                       runner->Update(3, QString::fromStdString("Unknown failure in analysis"));
                   }
               });
}

void  MainWindow::UpdateUi(const int& updateType, const QString& message)
{
    CorpusAnalyser::Log("UpdateUi " + std::to_string(updateType));
    if (updateType == 0)
    {
        this->setCursor(Qt::BusyCursor);
        mQMovie->start();
        ui->mLblGif->show();
        ui->mLblGif->repaint();
    }
    else if(updateType == 2 || updateType == 3)
    {
        this->setCursor(Qt::ArrowCursor);
        mQMovie->stop();
        ui->mLblGif->hide();
        ui->mLblGif->repaint();
    }
    ui->mBtnRun->setEnabled(updateType == 2 || updateType == 3);
    ui->mLstFilters->setEnabled(updateType == 2 || updateType == 3);
    ui->mEdtAxisX->setEnabled(updateType == 2 || updateType == 3);
    ui->mEdtAxisY->setEnabled(updateType == 2 || updateType == 3);
    ui->mEdtVowels->setEnabled(updateType == 2 || updateType == 3);
    ui->mActResultFolder->setEnabled(!ui->mEdtOutputDir->text().isEmpty());
    ui->mActExit->setEnabled(updateType == 2 || updateType == 3);
    this->statusBar()->showMessage(message);
}

void MainWindow::on_mBtnCorpus_clicked()
{
    auto corpus = QFileDialog::getOpenFileName(this,
                                               tr("Select a corpus..."),
                                               QApplication::applicationDirPath());
    ui->mEdtCorpus->setText(corpus);
}

void MainWindow::on_mBtnOutputDir_clicked()
{
    auto outputDir = QFileDialog::getExistingDirectory(this,
                                                       tr("Select an output directory..."),
                                                       QApplication::applicationDirPath());
    ui->mEdtOutputDir->setText(outputDir);
}

void MainWindow::on_mActExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_mActResultFolder_triggered()
{
    QString path = QDir::toNativeSeparators(ui->mEdtOutputDir->text());
    QDesktopServices::openUrl(QUrl("file:///" + path));
}

void MainWindow::on_mActAbout_triggered()
{
    const QString message(QString::fromStdString("<center>"
                                                 "Corpus Analyser<br/><br/>"
                                                 "Brookes Babylab<br/><br/>"
                                                 "&copy;2014<br/><br/>"
                                                 "Version " + CorpusAnalyser::kVersion +
                                                 "</center>"));
    QMessageBox::about(this, tr("About"), message);
}
