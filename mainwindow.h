#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "taskmanager.h"
#include <QCloseEvent>
#include "tasknotifier.h"
#include "mainwindow_ui.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void initData();
    QList<Task> getFilteredAndSortedTasks();

private slots:
    void addNewTask();
    void refreshTaskList();
    void onTaskStatusChanged(QString taskId, int state);
    void onDeleteTaskClicked(QString taskId);
    void onUndoTaskClicked();
    void onMenuButtonClicked();

private:
    Ui::MainWindow *ui;
    MainWindowUI *view;
    TaskNotifier *notifier = nullptr;

    TaskManager taskManager;
    QString dataFilePath;

    int currentStatusFilter = 0;   // 0: Tất cả, 1: Chưa làm, 2: Đang làm, 3: Hoàn thành
    int currentPriorityFilter = 0; // 0: Tất cả, giá trị số nhập vào sẽ lọc chính xác (Ví dụ: 1, 2, 3...)
    int currentSortOrder = 0;      // 0: Mặc định, 1: Sắp tới trước, 2: Xa nhất trước
};

#endif // MAINWINDOW_H