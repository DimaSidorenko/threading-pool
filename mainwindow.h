#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_rbFibonacci_clicked();

    void on_rbFactorial_clicked();

    void on_rbDoubleFactorial_clicked();

    void on_pbAddTask_clicked();

    void on_pbStartTaskQueue_clicked();

    void on_pbStopTaskQueue_clicked();

    void DrawTable();

private:
    Ui::MainWindow *ui;
    void CreateTable();
    void SetRow(QTableWidget* tw, int32_t index_row, size_t task_id, QString type, int64_t param,
                QString status, QString result);
    void UpdateButton();
};
#endif // MAINWINDOW_H
