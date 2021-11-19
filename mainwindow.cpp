#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <vector>
#include <QTimer>
#include "taskinfo.h"
#include "thread-pool.h"
#include "sample_tasks.h"


std::vector<TaskInfo> tasks;

QMap<TaskType, QString> convert_task_type_to_string;
QMap<TaskStatus, QString> convert_task_status_to_string;

bool thread_pool_started = false;


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{   
    ui->setupUi(this);
    ui->statusbar->showMessage("Задачи не запущены");


    convert_task_status_to_string[TaskStatus::waiting] = "waiting";
    convert_task_status_to_string[TaskStatus::done] = "done";
    convert_task_status_to_string[TaskStatus::in_progress] = "in progress";

    convert_task_type_to_string[TaskType::Fibonacci] = "Число Фибоначчи";
    convert_task_type_to_string[TaskType::Factorial] = "Факториал числа";
    convert_task_type_to_string[TaskType::DoubleFactorial] = "Двойной факториал числа";

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(DrawTable()));
    timer->start(300);

    //ui->twTasksQueue->insertRow(ui->twTasksQueue->rowCount());
    //SetRow(ui->twTasksQueue, 0,  1, "kek", 1, "kek", "kek");

    UpdateButton();
    CreateTable();
}



void MainWindow::CreateTable(){
    ui-> twTasksQueue->setColumnCount(5);
    ui->twTasksQueue->setColumnWidth(0, 120);
    ui->twTasksQueue->setColumnWidth(1, 120);
    ui->twTasksQueue->setColumnWidth(2, 120);
    ui->twTasksQueue->setColumnWidth(3, 120);
    ui->twTasksQueue->setColumnWidth(4, 120);

    //ui->twTasksQueue->setShowGrid(true);
    ui-> twTasksQueue->setSelectionBehavior(QAbstractItemView::SelectRows);  // Разрешаем выделение построчно

    QStringList labels = {"Id задачи", "Описание задачи", "N", "Статус", "Результат"};
    ui->twTasksQueue->setHorizontalHeaderLabels(labels);    // Устанавливаем заголовки колонок
}


enum ColumnLabels{
 taskId, Type, Param, Status, Result
};

void MainWindow::SetRow(QTableWidget* tw, int32_t index_row, size_t task_id, QString type, int64_t param,
                        QString status, QString result){

    {
        QTableWidgetItem* new_item = new QTableWidgetItem(QString::number(task_id));
        new_item->setTextAlignment(Qt::AlignCenter);
         tw->setItem(index_row, taskId, new_item);
    }
    {
        QTableWidgetItem* new_item = new QTableWidgetItem(type);
        new_item->setTextAlignment(Qt::AlignCenter);
        tw->setItem(index_row, Type, new_item);
    }
    {
        QTableWidgetItem* new_item = new QTableWidgetItem(QString::number(param));
        new_item->setTextAlignment(Qt::AlignCenter);
        tw->setItem(index_row, Param, new_item);
    }
    {
        QTableWidgetItem* new_item = new QTableWidgetItem(status);
        new_item->setTextAlignment(Qt::AlignCenter);
        tw->setItem(index_row, Status, new_item);
    }
    {
        QTableWidgetItem* new_item = new QTableWidgetItem(result);
        new_item->setTextAlignment(Qt::AlignCenter);
        tw->setItem(index_row, Result, new_item);
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_rbFibonacci_clicked()
{
    if (ui->rbFibonacci->isChecked()){
        ui->rbDoubleFactorial->setChecked(false);
        ui->rbFactorial->setChecked(false);
    }
}


void MainWindow::on_rbFactorial_clicked()
{
    if (ui->rbFactorial->isChecked()){
        ui->rbDoubleFactorial->setChecked(false);
        ui->rbFibonacci->setChecked(false);
    }
}


void MainWindow::on_rbDoubleFactorial_clicked()
{
    if (ui->rbDoubleFactorial->isChecked()){
        ui->rbFibonacci->setChecked(false);
        ui->rbFactorial->setChecked(false);
    }
}

void MainWindow::on_pbAddTask_clicked()
{
    int32_t task_id = ++task_id_counter;

    TaskType task_type;
    int32_t param;

    bool is_any_checked = false;
    if (ui->rbFibonacci->isChecked()){
        task_type = Fibonacci;
        param = ui->sbFibonacci->value();
        is_any_checked = true;
    }
    if (ui->rbFactorial->isChecked()){
        task_type = Factorial;
        param = ui->sbFactorial->value();
        is_any_checked = true;
    }
    if (ui->rbDoubleFactorial->isChecked()){
        task_type = DoubleFactorial;
        param = ui->sbDoubleFactorial->value();
        is_any_checked = true;
    }

    if (is_any_checked){
        tasks.push_back(TaskInfo(task_id, task_type, param));
    }
    else {
       QMessageBox::about(this, "Ошибка", "Вы не выбрали задачу");
    }
}


void MainWindow::on_pbStartTaskQueue_clicked()
{
    thread_pool_started = true;
    UpdateButton();


    thread_pool& pool = thread_pool::instance();

    for(size_t i = 0; i < tasks.size(); i++){
        int32_t task_id = tasks[i].task_id;

        //if task completed we don't add it in tasks queue
        if (pool.get_task_status(task_id) == TaskStatus::done)
            continue;

        switch(tasks[i].task_type)
        {
            case Fibonacci:
                pool.add_task(fibbonacci, tasks[i].param, tasks[i].task_id);
                break;
            case Factorial:
                pool.add_task(factorial, tasks[i].param, tasks[i].task_id);
                break;
            case DoubleFactorial:
                pool.add_task(double_factorial, tasks[i].param, tasks[i].task_id);
                break;
        }
    }

    int32_t count_threads = ui->sbCountThreads->value();

    pool.start_pool(count_threads);
}


void MainWindow::DrawTable(){
    QTableWidget* tasks_queue = ui->twTasksQueue;
    tasks_queue->setRowCount(0);

    thread_pool& pool = thread_pool::instance();

    ui->statusbar->showMessage(QString::number(tasks_queue->rowCount()));

    for(size_t i = 0; i < tasks.size(); i++){
        int32_t task_id = tasks[i].task_id;

        QString task_result = QString::fromStdString(pool.get_task_result(task_id)); // in the way when task waiting or in_progress;

        QString task_status = convert_task_status_to_string[pool.get_task_status(task_id)];

        int64_t task_param = tasks[i].param;

        QString task_type = convert_task_type_to_string[tasks[i].task_type];

        tasks_queue->insertRow(tasks_queue->rowCount());
        SetRow(tasks_queue, i, task_id, task_type, task_param, task_status, task_result);
    }
}


void MainWindow::on_pbStopTaskQueue_clicked()
{
    thread_pool_started = false;
    UpdateButton();

    thread_pool& pool = thread_pool::instance();

    pool.stop_pool();

    while(!tasks.empty()){
        int32_t task_id = tasks.back().task_id;
        //TO-DO add operator != for TaskInfo and rewrite it using this operator
        if (pool.get_task_result(task_id) == "---"){
            tasks.pop_back();
        }
        else {
            break;
        }
    }
}


void MainWindow::UpdateButton(){
    if (thread_pool_started){
        ui->pbStartTaskQueue->setEnabled(false);
        ui->pbAddTask->setEnabled(false);
        ui->pbStopTaskQueue->setEnabled(true);
    }
    else {
        ui->pbStartTaskQueue->setEnabled(true);
        ui->pbAddTask->setEnabled(true);
        ui->pbStopTaskQueue->setEnabled(false);
    }
}





