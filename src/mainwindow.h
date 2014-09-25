#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QMovie>

namespace Ui { class MainWindow; }


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

public slots:
    void UpdateUi(const int& updateType, const QString &message);

private slots:

    void on_mBtnRun_clicked();

    void on_mBtnCorpus_clicked();

    void on_mBtnOutputDir_clicked();

    void on_mActExit_triggered();

    void on_mActResultFolder_triggered();

    void on_mActAbout_triggered();

private:
    Ui::MainWindow* ui;
    std::unique_ptr<QMovie> mQMovie;
};

#endif // MAINWINDOW_H
