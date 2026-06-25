#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <Qwidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QCheckBox>
#include "taskmanager.h"


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

private:
    void setupUI();
    void initData();

private slots:
    void addNewTask();
    void refreshTaskList();
    void clearLayout(QLayout *layout);
    QList<Task> getFilteredAndSortedTasks();
    void renderTaskItem(const Task &task);
    void onTaskStatusChanged(int state);
    void openTaskDetails();
    void onDeleteTaskClicked();
    void onUndoTaskClicked();
    void onMenuButtonClicked();

private:
    Ui::MainWindow *ui;

    // 2. Khai báo con trỏ nút bấm ở mục private
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QWidget *topBar;
    QWidget *contentArea;
    QScrollArea *scrollArea;
    QWidget *scrollContent;
    QVBoxLayout *contentLayout;
    QLabel *titleLabel;
    QHBoxLayout *topBarLayout;
    QToolButton *menuButton;
    QPushButton *newButton;
    
    QWidget *bottomBar;
    QHBoxLayout *bottomBarLayout;
    QPushButton *undoButton;

    TaskManager taskManager;
    QString dataFilePath;

    int currentStatusFilter = 0;   // 0: Tất cả, 1: Chưa làm, 2: Đang làm, 3: Hoàn thành
    int currentPriorityFilter = 0; // 0: Tất cả, giá trị số nhập vào sẽ lọc chính xác (Ví dụ: 1, 2, 3...)
    int currentSortOrder = 0;      // 0: Mặc định, 1: Sắp tới trước, 2: Xa nhất trước
};

#endif // MAINWINDOW_H