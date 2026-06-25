#include "mainwindow.h"
#include "filterdialog.h"
#include "apiservice.h"
#include "./ui_mainwindow.h"
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <algorithm>
#include <QTextEdit>
#include "taskdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Taskline");
    if (ui->menubar) {
        ui->menubar->hide();
    }

    setupUI();
    initData();

    connect(newButton, &QPushButton::clicked, this, &MainWindow::addNewTask);
    connect(undoButton, &QPushButton::clicked, this, &MainWindow::onUndoTaskClicked);
    connect(menuButton, &QToolButton::clicked, this, &MainWindow::onMenuButtonClicked);

    refreshTaskList();
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. Khởi tạo topBar và Layout ngang
    topBar = new QWidget(this);
    topBar->setStyleSheet("background-color: #2c3e50;");

    topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(15, 0, 15, 0); 
    topBarLayout->setSpacing(15);                  

    // 2. Khởi tạo nút Menu
    menuButton = new QToolButton(topBar);
    menuButton->setText("≡");
    menuButton->setStyleSheet(
        "QToolButton {"
        "   color: white; font-size: 40px; background: transparent;"
        "   border: none; padding-bottom: 4px;"
        "}"
        "QToolButton:hover { color: #3498db; }"
        );

    // 3. Khởi tạo nhãn Taskline
    titleLabel = new QLabel("Taskline", topBar);
    titleLabel->setStyleSheet(
        "color: white; font-size: 22px; font-weight: bold;"
        "font-family: 'Segoe UI', sans-serif; border: none;"
        );

    topBarLayout->addWidget(menuButton); 
    topBarLayout->addWidget(titleLabel); 
    topBarLayout->addStretch();

    // 4. Khởi tạo nút + New
    newButton = new QPushButton("+ New", topBar);
    newButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db; color: white; border-radius: 13px;"
        "   font-weight: bold; font-size: 15px; border: none;"
        "   min-width: 60px; min-height: 38px; padding: 0px 20px;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1c5980; }"
        );
    topBarLayout->addWidget(newButton);

    // 5. ScrollArea
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); 
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: #ecf0f1; border: none; }"
        "QScrollBar:vertical {"
        "   border: none; background: #e2e8f0; width: 8px; margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #cbd5e1; border-radius: 4px; min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #94a3b8; }"
        );

    scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;"); 
    
    contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(20, 20, 20, 20); 
    contentLayout->setSpacing(15);                     
    contentLayout->setAlignment(Qt::AlignTop);
    
    scrollArea->setWidget(scrollContent);

    // 6. BottomBar
    bottomBar = new QWidget(this);
    bottomBar->setStyleSheet("background-color: #ecf0f1;");
    bottomBarLayout = new QHBoxLayout(bottomBar);
    bottomBarLayout->setContentsMargins(15, 10, 15, 10);
    
    undoButton = new QPushButton("⟲ Undo", bottomBar);
    undoButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #f39c12; color: white; font-weight: bold; font-size: 14px;"
        "   border-radius: 6px; padding: 6px 15px;"
        "}"
        "QPushButton:hover { background-color: #e67e22; }"
        );
    bottomBarLayout->addStretch();
    bottomBarLayout->addWidget(undoButton);

    // Ghép Layout
    mainLayout->addWidget(topBar, 0);      
    mainLayout->addWidget(scrollArea, 1);
    mainLayout->addWidget(bottomBar, 0);
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
    if (dialog.exec() == QDialog::Accepted) {
        taskManager.addTask(
            dialog.getTitle(),
            dialog.getDescription(),
            dialog.getStatus(),
            dialog.getPriority(),
            dialog.getDeadline()
            );
        taskManager.saveToFile(dataFilePath);

        // Lấy task vừa tạo (nằm cuối cùng trong danh sách)
        QList<Task> all = taskManager.getAllTasks();
        if (!all.isEmpty()) {
            TaskStats ts;
            ts.id = all.last().getId();
            ts.title = dialog.getTitle();
            ts.description = dialog.getDescription();
            ts.status = dialog.getStatus();
            ts.priority = dialog.getPriority();
            ts.deadline = dialog.getDeadline();
            APIService::instance().createNewTask(ts, [](bool success, QJsonArray data){
                // Xử lý khi API phản hồi (không bắt buộc vì dùng local làm chính hiện tại)
            });
        }
        
        refreshTaskList();
    }
}

void MainWindow::refreshTaskList() {
    clearLayout(contentLayout);

    QList<Task> finalTasks = getFilteredAndSortedTasks();

    for (const Task &task : finalTasks) {
        renderTaskItem(task);
    }
}

void MainWindow::clearLayout(QLayout *layout) {
    if (!layout) return;

    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

QList<Task> MainWindow::getFilteredAndSortedTasks() {
    QList<Task> displayTasks;

    if (currentStatusFilter == 1) displayTasks = taskManager.filterByStatus(TaskStatus::TODO);
    else if (currentStatusFilter == 2) displayTasks = taskManager.filterByStatus(TaskStatus::IN_PROGRESS);
    else if (currentStatusFilter == 3) displayTasks = taskManager.filterByStatus(TaskStatus::DONE);
    else displayTasks = taskManager.getAllTasks();

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

QWidget* MainWindow::createTextContainer(const Task &task, QWidget *parent) {
    QWidget *textContainer = new QWidget(parent);
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(5);

    QLabel *nameLabel = new QLabel(task.getTitle(), textContainer);
    QString textStyle = "color: #2c3e50; font-size: 18px; font-weight: bold; background: transparent;";
    if (task.getStatus() == TaskStatus::DONE) {
        textStyle += " text-decoration: line-through; color: #95a5a6;";
    }
    nameLabel->setStyleSheet(textStyle);

    QTextEdit *descriptionLabel = createDescriptionLabel(task.getDescription(), textContainer);

    QLabel *priorityLabel = new QLabel("Độ ưu tiên: " + QString::number(task.getPriority()), textContainer);
    priorityLabel->setStyleSheet("font-size: 13px; color: #2f3542; font-weight: 500; background: transparent;");

    QString dtStr = task.getDeadline().toString("dd/MM/yyyy hh:mm");
    QLabel *deadlineLabel = new QLabel("Deadline: " + dtStr, textContainer);
    QString deadlineStyle = "font-size: 13px; background: transparent;";
    if (task.isOverdue()) {
        deadlineStyle += " color: #e74c3c; font-weight: bold;";
    } else {
        deadlineStyle += " color: #7f8c8d;";
    }
    deadlineLabel->setStyleSheet(deadlineStyle);

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(descriptionLabel);
    textLayout->addWidget(priorityLabel);
    textLayout->addWidget(deadlineLabel);

    return textContainer;
}

QTextEdit* MainWindow::createDescriptionLabel(const QString &desc, QWidget *parent) {
    QTextEdit *descriptionLabel = new QTextEdit(parent);
    descriptionLabel->setStyleSheet("font-size: 14px; color: #57606f; background: transparent; border: none;");
    descriptionLabel->setWordWrapMode(QTextOption::WrapAnywhere);
    descriptionLabel->setReadOnly(true);
    descriptionLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    descriptionLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    descriptionLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    descriptionLabel->document()->setDocumentMargin(0);
    descriptionLabel->setMaximumHeight(65);
    descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QString rawDesc = desc.trimmed();
    if (rawDesc.isEmpty()) {
        descriptionLabel->setText("(Không có mô tả)");
    } else {
        int maxChars = 115;
        if (rawDesc.length() > maxChars) {
            descriptionLabel->setText(rawDesc.left(maxChars).trimmed() + "...");
        } else {
            descriptionLabel->setText(rawDesc);
        }
    }
    return descriptionLabel;
}

QWidget* MainWindow::createStatusBadge(TaskStatus status, QWidget *parent) {
    QWidget *statusColorWidget = new QWidget(parent);
    QString statusColor = "#95a5a6";
    if (status == TaskStatus::TODO) statusColor = "#e74c3c";
    else if (status == TaskStatus::IN_PROGRESS) statusColor = "#f1c40f";
    else if (status == TaskStatus::DONE) statusColor = "#2ecc71";

    statusColorWidget->setStyleSheet(QString("background-color: %1; border-radius: 6px;").arg(statusColor));
    statusColorWidget->setFixedSize(12, 12);
    return statusColorWidget;
}

QToolButton* MainWindow::createDeleteButton(int taskId, QWidget *parent) {
    QToolButton *deleteBtn = new QToolButton(parent);
    deleteBtn->setText("🗑");
    deleteBtn->setStyleSheet(
        "QToolButton { color: #e74c3c; font-size: 18px; background: transparent; border: none; }"
        "QToolButton:hover { color: #c0392b; font-size: 20px; }"
    );
    deleteBtn->setProperty("taskId", taskId);
    connect(deleteBtn, &QToolButton::clicked, this, &MainWindow::onDeleteTaskClicked);
    return deleteBtn;
}

QCheckBox* MainWindow::createStatusCheckBox(const Task &task, QWidget *parent) {
    QCheckBox *checkBox = new QCheckBox(parent);
    checkBox->setStyleSheet("QCheckBox::indicator { width: 20px; height: 20px; }");
    checkBox->setChecked(task.getStatus() == TaskStatus::DONE);
    checkBox->setProperty("taskId", task.getId());
    connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::onTaskStatusChanged);
    return checkBox;
}

void MainWindow::renderTaskItem(const Task &task) {
    QWidget *taskItem = new QWidget(scrollContent);
    taskItem->setObjectName("taskItem");
    taskItem->setStyleSheet(
        "QWidget#taskItem {"
        "   background-color: white; border-radius: 10px;"
        "   border: 1px solid #dcdde1; min-height: 60px;"
        "}"
        "QWidget#taskItem:hover {"
        "   border: 1px solid #3498db; background-color: #f8fbfc;"
        "}"
    );

    QHBoxLayout *taskLayout = new QHBoxLayout(taskItem);
    taskLayout->setContentsMargins(15, 10, 15, 10);

    QWidget *textContainer = createTextContainer(task, taskItem);
    QWidget *statusBadge = createStatusBadge(task.getStatus(), taskItem);
    QToolButton *deleteBtn = createDeleteButton(task.getId(), taskItem);
    QCheckBox *checkBox = createStatusCheckBox(task, taskItem);

    taskLayout->addWidget(textContainer, 1);
    taskLayout->addStretch();
    taskLayout->addWidget(statusBadge);
    taskLayout->addWidget(deleteBtn);
    taskLayout->addWidget(checkBox);

    taskItem->setProperty("taskId", task.getId());
    taskItem->setCursor(Qt::PointingHandCursor);
    taskItem->installEventFilter(this);

    contentLayout->addWidget(taskItem);
}

void MainWindow::onTaskStatusChanged(int state)
{
    QCheckBox *senderCb = qobject_cast<QCheckBox*>(sender());
    if (senderCb) {
        int taskId = senderCb->property("taskId").toInt();
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
            taskManager.editTask(taskId, targetTask.getTitle(), targetTask.getDescription(), TaskStatus::TODO, targetTask.getPriority(), targetTask.getDeadline());
            targetTask.setStatus(TaskStatus::TODO);
        }
        taskManager.saveToFile(dataFilePath);

        // API Sync
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
}

void MainWindow::onDeleteTaskClicked()
{
    QToolButton *senderBtn = qobject_cast<QToolButton*>(sender());
    if (senderBtn) {
        int taskId = senderBtn->property("taskId").toInt();
        taskManager.deleteTask(taskId);
        taskManager.saveToFile(dataFilePath);

        // API Sync
        APIService::instance().deleteTask(taskId, [](bool, QJsonArray){});

        refreshTaskList();
    }
}

void MainWindow::onUndoTaskClicked()
{
    taskManager.undoDelete();
    taskManager.saveToFile(dataFilePath);

    // API Sync - Undo deletes the most recently deleted task from local trash
    // We get the restored task from the manager to send to API
    QList<Task> all = taskManager.getAllTasks();
    if (!all.isEmpty()) {
        // Giả sử Task vừa Undo được push vào cuối (theo cơ chế m_tasks của TaskManager)
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

void MainWindow::openTaskDetails()
{
    QWidget *senderWidget = qobject_cast<QWidget*>(sender());
    // Mặc dù eventFilter xử lý, nhưng để gọi được slot cần gán tín hiệu hoặc gọi trực tiếp từ filter.
}

// Bổ sung hàm eventFilter cho MainWindow (cần khai báo override trong .h)
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget *widget = qobject_cast<QWidget*>(watched);
        if (widget && widget->property("taskId").isValid()) {
            int taskId = widget->property("taskId").toInt();
            
            QList<Task> allTasks = taskManager.getAllTasks();
            for (const Task &t : allTasks) {
                if (t.getId() == taskId) {
                    TaskDialog dialog(this);
                    dialog.setTaskData(t);
                    if (dialog.exec() == QDialog::Accepted) {
                        taskManager.editTask(
                            taskId,
                            dialog.getTitle(),
                            dialog.getDescription(),
                            dialog.getStatus(),
                            dialog.getPriority(),
                            dialog.getDeadline()
                            );
                        taskManager.saveToFile(dataFilePath);

                        // API Sync
                        TaskStats ts;
                        ts.id = taskId;
                        ts.title = dialog.getTitle();
                        ts.description = dialog.getDescription();
                        ts.status = dialog.getStatus();
                        ts.priority = dialog.getPriority();
                        ts.deadline = dialog.getDeadline();
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
    // Truyền các giá trị lọc hiện tại vào để hiển thị lại trên dialog
    FilterDialog dialog(currentStatusFilter, currentPriorityFilter, currentSortOrder, this);

    if (dialog.exec() == QDialog::Accepted) {
        currentStatusFilter = dialog.getSelectedStatus();
        currentPriorityFilter = dialog.getSelectedPriority();
        currentSortOrder = dialog.getSortOrder();

        // Làm mới lại danh sách hiển thị với bộ lọc mới
        refreshTaskList();
    }
}