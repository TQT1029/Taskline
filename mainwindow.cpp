#include "mainwindow.h"
#include "filterdialog.h"
#include "apiservice.h"
#include "./ui_mainwindow.h"
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <algorithm>
#include "taskdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , view(new MainWindowUI(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Taskline");
    this->setMaximumWidth(660);
    this->setMinimumWidth(380);
    if (ui->menubar) {
        ui->menubar->hide();
    }

    view->setupUI(this);
    initData();

    connect(view->newButton, &QPushButton::clicked, this, &MainWindow::addNewTask);
    connect(view->undoButton, &QPushButton::clicked, this, &MainWindow::onUndoTaskClicked);
    connect(view->menuButton, &QToolButton::clicked, this, &MainWindow::onMenuButtonClicked);
    connect(view->taskSearch, &TaskSearchHelper::searchRequested, this, &MainWindow::refreshTaskList);
    
    connect(view, &MainWindowUI::deleteTaskClicked, this, &MainWindow::onDeleteTaskClicked);
    connect(view, &MainWindowUI::taskStatusChanged, this, &MainWindow::onTaskStatusChanged);

    refreshTaskList();

    notifier = new TaskNotifier(this);

    QTimer *systemTimer = new QTimer(this);
    connect(systemTimer, &QTimer::timeout, this, [this]() {
        if (notifier) {
            notifier->checkDeadlines(taskManager.getAllTasks());
        }
    });
    systemTimer->start(10000); // Khởi động quét 10 giây/lần
}

void MainWindow::initData()
{
    dataFilePath = QDir::currentPath() + "/tasks.json";
    taskManager.loadFromFile(dataFilePath);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addNewTask()
{
    TaskDialog dialog(this);
    dialog.setDefaultDeadline(QDateTime::currentDateTime());

    if (dialog.exec() == QDialog::Accepted) {
        QDateTime userDateTime = dialog.getDeadline().getDateTime();

        taskManager.addTask(
            dialog.getTitle(),
            dialog.getDescription(),
            TaskStatus::IN_PROGRESS,
            dialog.getPriority(),
            userDateTime
            );
        taskManager.saveToFile(dataFilePath);

        QList<Task> all = taskManager.getAllTasks();
        if (!all.isEmpty()) {
            TaskStats ts;
            ts.id = all.last().getId();
            ts.title = dialog.getTitle();
            ts.description = dialog.getDescription();
            ts.status = TaskStatus::IN_PROGRESS;
            ts.priority = dialog.getPriority();
            ts.deadline = userDateTime;

            APIService::instance().createNewTask(ts, [](bool success, QJsonArray data){
                Q_UNUSED(success);
                Q_UNUSED(data);
            });
        }

        refreshTaskList();
    }
}

void MainWindow::refreshTaskList() {
    view->clearLayout();

    QList<Task> finalTasks = getFilteredAndSortedTasks();
    QString keyword = view->taskSearch->searchEdit ? view->taskSearch->searchEdit->text().trimmed() : "";

    for (const Task &task : finalTasks) {
        if (keyword.isEmpty() || task.getTitle().contains(keyword, Qt::CaseInsensitive)
            || task.getDescription().contains(keyword, Qt::CaseInsensitive)) {
            view->renderTaskItem(task, this);
        }
    }
    view->taskTracker->updateStatistics(taskManager.getAllTasks());
}

QList<Task> MainWindow::getFilteredAndSortedTasks() {
    QList<Task> allTasks = taskManager.getAllTasks();
    QList<Task> displayTasks;
    QDateTime now = QDateTime::currentDateTime();

    for (Task &task : allTasks) {
        if (task.getStatus() != TaskStatus::DONE) {
            if (now > task.getDeadline()) {
                if (task.getStatus() != TaskStatus::TODO) {
                    task.setStatus(TaskStatus::TODO);
                    taskManager.editTask(task.getId(), task.getTitle(), task.getDescription(), TaskStatus::TODO, task.getPriority(), task.getDeadline());
                }
            } else {
                if (task.getStatus() != TaskStatus::IN_PROGRESS) {
                    task.setStatus(TaskStatus::IN_PROGRESS);
                    taskManager.editTask(task.getId(), task.getTitle(), task.getDescription(), TaskStatus::IN_PROGRESS, task.getPriority(), task.getDeadline());
                }
            }
        }
    }
    taskManager.saveToFile(dataFilePath);

    if (currentStatusFilter == 1) {
        for (const Task &t : allTasks) { if (t.getStatus() == TaskStatus::TODO) displayTasks.append(t); }
    } else if (currentStatusFilter == 2) {
        for (const Task &t : allTasks) { if (t.getStatus() == TaskStatus::IN_PROGRESS) displayTasks.append(t); }
    } else if (currentStatusFilter == 3) {
        for (const Task &t : allTasks) { if (t.getStatus() == TaskStatus::DONE) displayTasks.append(t); }
    } else {
        displayTasks = allTasks;
    }

    if (currentPriorityFilter != 0) {
        QList<Task> tempTasks;
        for (const Task &task : displayTasks) {
            if (task.getPriority() == currentPriorityFilter) {
                tempTasks.append(task);
            }
        }
        displayTasks = tempTasks;
    }

    if (currentSortOrder == 1) {
        std::sort(displayTasks.begin(), displayTasks.end(), [](const Task &a, const Task &b) {
            return a.getDeadline() < b.getDeadline();
        });
    } else if (currentSortOrder == 2) {
        std::sort(displayTasks.begin(), displayTasks.end(), [](const Task &a, const Task &b) {
            return a.getDeadline() > b.getDeadline();
        });
    }

    return displayTasks;
}

void MainWindow::onTaskStatusChanged(QString taskId, int state)
{
    QList<Task> allTasks = taskManager.getAllTasks();
    Task targetTask = allTasks.first();

    for (const Task &t : allTasks) {
        if (t.getId() == taskId) {
            targetTask = t;
            break;
        }
    }

    if (state == Qt::Checked) {
        taskManager.markTaskDone(taskId);
        targetTask.setStatus(TaskStatus::DONE);
    } else {
        taskManager.editTask(
            taskId,
            targetTask.getTitle(),
            targetTask.getDescription(),
            TaskStatus::IN_PROGRESS,
            targetTask.getPriority(),
            targetTask.getDeadline()
            );
        targetTask.setStatus(TaskStatus::IN_PROGRESS);
    }
    taskManager.saveToFile(dataFilePath);

    TaskStats ts;
    ts.id = targetTask.getId();
    ts.title = targetTask.getTitle();
    ts.description = targetTask.getDescription();
    ts.status = targetTask.getStatus();
    ts.priority = targetTask.getPriority();
    ts.deadline = targetTask.getDeadline();

    APIService::instance().updateTask(ts, [](bool, QJsonArray){});

    refreshTaskList();
}

void MainWindow::onDeleteTaskClicked(QString taskId)
{
    taskManager.deleteTask(taskId);
    taskManager.saveToFile(dataFilePath);

    APIService::instance().deleteTask(taskId, [](bool, QJsonArray){});

    refreshTaskList();
}

void MainWindow::onUndoTaskClicked()
{
    taskManager.undoDelete();
    taskManager.saveToFile(dataFilePath);

    QList<Task> all = taskManager.getAllTasks();
    if (!all.isEmpty()) {
        Task targetTask = all.last();
        TaskStats ts;
        ts.id = targetTask.getId();
        ts.title = targetTask.getTitle();
        ts.description = targetTask.getDescription();
        ts.status = targetTask.getStatus();
        ts.priority = targetTask.getPriority();
        ts.deadline = targetTask.getDeadline();

        APIService::instance().createNewTask(ts, [](bool, QJsonArray){});
    }

    refreshTaskList();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget *widget = qobject_cast<QWidget*>(watched);
        if (widget && widget->property("taskId").isValid()) {
            QString taskId = widget->property("taskId").toString();

            QList<Task> allTasks = taskManager.getAllTasks();
            for (const Task &t : allTasks) {
                if (t.getId() == taskId) {
                    TaskDialog dialog(this);
                    dialog.setTaskData(t);
                    if (dialog.exec() == QDialog::Accepted) {
                        QDateTime updatedDateTime = dialog.getDeadline().getDateTime();

                        taskManager.editTask(
                            taskId,
                            dialog.getTitle(),
                            dialog.getDescription(),
                            dialog.getStatus(),
                            dialog.getPriority(),
                            updatedDateTime
                            );
                        taskManager.saveToFile(dataFilePath);

                        TaskStats ts;
                        ts.id = taskId;
                        ts.title = dialog.getTitle();
                        ts.description = dialog.getDescription();
                        ts.status = dialog.getStatus();
                        ts.priority = dialog.getPriority();
                        ts.deadline = updatedDateTime;

                        APIService::instance().updateTask(ts, [](bool, QJsonArray){});

                        refreshTaskList();
                    }
                    break;
                }
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onMenuButtonClicked()
{
    FilterDialog dialog(currentStatusFilter, currentPriorityFilter, currentSortOrder, this);

    if (dialog.exec() == QDialog::Accepted) {
        currentStatusFilter = dialog.getSelectedStatus();
        currentPriorityFilter = dialog.getSelectedPriority();
        currentSortOrder = dialog.getSortOrder();

        refreshTaskList();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (notifier) {
        this->hide();

        notifier->sendNotification(
            "Taskline vẫn đang chạy ngầm",
            "Ứng dụng đã được thu nhỏ xuống khay hệ thống để tiếp tục theo dõi deadline.",
            QSystemTrayIcon::Information,
            2000
            );

        event->ignore();
    } else {
        event->accept();
    }
}