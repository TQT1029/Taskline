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
#include "ThemeUtils.h"
#include "taskstatusbar.h"
#include "tasksearch.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Taskline");
    this->setMaximumWidth(660);
    this->setMinimumWidth(380);
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
    topBar->setStyleSheet(QString("background-color: %1; border-bottom: 1px solid %2;").arg(ThemeUtils::bgBar(), ThemeUtils::border()));

    topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(15, 0, 15, 0); 
    topBarLayout->setSpacing(15);                  

    // 2. Khởi tạo nút Menu
    menuButton = new QToolButton(topBar);
    menuButton->setText("≡");
    menuButton->setStyleSheet(QString(
                                  "QToolButton {"
                                  "   color: %1; font-size: 40px; background: transparent;"
                                  "   border: none; padding-bottom: 4px;"
                                  "}"
                                  "QToolButton:hover { color: %2; }"
                                  ).arg(ThemeUtils::textMain(), ThemeUtils::btnPrimary()));

    // 3. Khởi tạo nhãn Taskline
    titleLabel = new QLabel("Taskline", topBar);
    titleLabel->setStyleSheet(QString(
                                  "color: %1; font-size: 22px; font-weight: bold;"
                                  "font-family: 'Segoe UI', sans-serif; border: none;"
                                  ).arg(ThemeUtils::textMain()));

    topBarLayout->addWidget(menuButton); 
    topBarLayout->addWidget(titleLabel);

    topBarLayout->addStretch();

    taskSearch.setupInWidget(topBar);
    topBarLayout->addWidget(taskSearch.searchButton);
    topBarLayout->addWidget(taskSearch.searchEdit);

    connect(&taskSearch, &TaskSearchHelper::searchRequested, this, &MainWindow::refreshTaskList);

    // 4. Khởi tạo nút + New
    newButton = new QPushButton("+ New", topBar);
    newButton->setStyleSheet(QString(
                                 "QPushButton {"
                                 "   background-color: %1; color: %2; border-radius: 13px;"
                                 "   font-weight: bold; font-size: 15px; border: none;"
                                 "   min-width: 60px; min-height: 38px; padding: 0px 20px;"
                                 "}"
                                 "QPushButton:hover { background-color: %3; }"
                                 ).arg(ThemeUtils::btnPrimary(), ThemeUtils::btnPrimaryText(), ThemeUtils::btnPrimaryHover()));
    topBarLayout->addWidget(newButton);

    // 5. ScrollArea
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); 
    scrollArea->setStyleSheet(QString(
                                  "QScrollArea { background-color: %1; border: none; }"
                                  ).arg(ThemeUtils::bgApp()));

    scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;"); 
    
    contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(20, 20, 20, 20); 
    contentLayout->setSpacing(15);                     
    contentLayout->setAlignment(Qt::AlignTop);
    
    scrollArea->setWidget(scrollContent);

    // 6. BottomBar
    bottomBar = new QWidget(this);
    bottomBar->setStyleSheet(QString("background-color: %1; border-top: 1px solid %2;").arg(ThemeUtils::bgBar(), ThemeUtils::border()));
    bottomBarLayout = new QHBoxLayout(bottomBar);
    bottomBarLayout->setContentsMargins(15, 10, 15, 10);
    
    undoButton = new QPushButton("⟲ Undo", bottomBar);
    undoButton->setStyleSheet(QString(
                                  "QPushButton {"
                                  "   background-color: %1; color: %2; font-weight: bold; font-size: 14px;"
                                  "   border: 1px solid %3; border-radius: 6px; padding: 6px 15px;"
                                  "}"
                                  "QPushButton:hover { background-color: %4; }"
                                  ).arg(ThemeUtils::btnSecondary(), ThemeUtils::btnSecondaryText(), ThemeUtils::border(), ThemeUtils::btnSecondaryHover()));
    taskTracker.setupInWidget(bottomBar);
    bottomBarLayout->addWidget(taskTracker.statusLabel);
    bottomBarLayout->addWidget(taskTracker.progressBar);

    bottomBarLayout->addSpacing(15);

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

    QString keyword = taskSearch.searchEdit ? taskSearch.searchEdit->text().trimmed() : "";

    finalTasks = getFilteredAndSortedTasks();
    for (const Task &task : finalTasks) {
        // CHỈ VẼ RA MÀN HÌNH NẾU: Ô search trống HOẶC Tiêu đề/Mô tả có chứa từ khóa
        if (keyword.isEmpty() || task.getTitle().contains(keyword, Qt::CaseInsensitive)  //
            || task.getDescription().contains(keyword, Qt::CaseInsensitive)) { //
            renderTaskItem(task); //
        }
    }
    taskTracker.updateStatistics(taskManager.getAllTasks());
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
    QString textStyle = QString("color: %1; font-size: 18px; font-weight: bold; background: transparent;").arg(ThemeUtils::textMain());
    if (task.getStatus() == TaskStatus::DONE) {
        textStyle += QString(" text-decoration: line-through; color: %1;").arg(ThemeUtils::textDone());
    }
    nameLabel->setStyleSheet(textStyle);

    QLabel *descriptionLabel = createDescriptionLabel(task.getDescription(), textContainer);

    QLabel *priorityLabel = new QLabel("Độ ưu tiên: " + QString::number(task.getPriority()), textContainer);
    priorityLabel->setStyleSheet(QString("font-size: 13px; color: %1; font-weight: 500; background: transparent;").arg(ThemeUtils::textSub()));

    QString dtStr = task.getDeadline().toString("dd/MM/yyyy hh:mm");
    QLabel *deadlineLabel = new QLabel("Deadline: " + dtStr, textContainer);
    QString deadlineStyle = "font-size: 13px; background: transparent;";
    if (task.isOverdue()) {
        deadlineStyle += " color: #e74c3c; font-weight: bold;";
    } else {
        deadlineStyle += QString(" color: %1;").arg(ThemeUtils::textSub());
    }
    deadlineLabel->setStyleSheet(deadlineStyle);

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(descriptionLabel);
    textLayout->addWidget(priorityLabel);
    textLayout->addWidget(deadlineLabel);

    return textContainer;
}

QLabel* MainWindow::createDescriptionLabel(const QString &desc, QWidget *parent) {
    QLabel *descriptionLabel = new QLabel(parent);
    descriptionLabel->setStyleSheet(QString("font-size: 14px; color: %1; background: transparent; border: none;").arg(ThemeUtils::textSub()));
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QString rawDesc = desc.trimmed();
    if (rawDesc.isEmpty()) {
        descriptionLabel->setText("(Không có mô tả)");
    } else {
        int maxChars = 115;
        QString displayDesc = rawDesc;
        if (displayDesc.length() > maxChars) {
            displayDesc = displayDesc.left(maxChars).trimmed() + "...";
        }

        // Chèn zero-width space (\u200B) để QLabel có thể tự do wrap/break-word
        // đối với các chuỗi dài liên tục không có dấu cách (như "awwwgydgyddd...")
        // mà vẫn giữ nguyên khả năng co giãn chiều cao (auto-scale height).
        QString wrappedDesc;
        for (int i = 0; i < displayDesc.length(); ++i) {
            wrappedDesc += displayDesc[i];
            if (displayDesc[i] != ' ') {
                wrappedDesc += QChar(0x200B);
            }
        }
        descriptionLabel->setText(wrappedDesc);
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

QToolButton* MainWindow::createDeleteButton(QString taskId, QWidget *parent) {
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
    // Tăng kích thước bộ chỉ thị (ô vuông) để nó nổi bật hơn mà vẫn giữ nguyên giao diện gốc của OS (tránh mất dấu tick)
    checkBox->setStyleSheet(QString(
        "QCheckBox::indicator {"
        "   width: 22px;"
        "   height: 22px;"
        "   border-radius: 5px;"
        "   border: 2px solid #bdc3c7;"
        "   background-color: %1;"
        "}"


        "QCheckBox::indicator:hover {"
        "   border: 2px solid #3498db;"
        "}"

        "QCheckBox::indicator:checked {"
        "   background-color: %2;"
        "   border: 2px solid #2ecc71;"
        "   image: url(:/Assets/icons/check.png)"
        "}").arg(ThemeUtils::checkBoxBgNormal(), ThemeUtils::checkBoxBgChecked()));
    checkBox->setChecked(task.getStatus() == TaskStatus::DONE);
    checkBox->setProperty("taskId", task.getId());
    connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::onTaskStatusChanged);
    return checkBox;
}

void MainWindow::renderTaskItem(const Task &task) {
    QWidget *taskItem = new QWidget(scrollContent);
    taskItem->setObjectName("taskItem");
    taskItem->setStyleSheet(QString(
                                "QWidget#taskItem {"
                                "   background-color: %1; border-radius: 10px;"
                                "   border: 1px solid %2; height: auto;"
                                "}"
                                "QWidget#taskItem:hover {"
                                "   border: 1px solid %3; background-color: %4;"
                                "}"
                                ).arg(ThemeUtils::itemBg(), ThemeUtils::itemBorder(), ThemeUtils::itemHoverBorder(), ThemeUtils::itemHoverBg()));

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
        QString taskId = senderCb->property("taskId").toString();
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
        QString taskId = senderBtn->property("taskId").toString();
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
            QString taskId = widget->property("taskId").toString();
            
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