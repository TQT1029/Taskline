#include "mainwindow_ui.h"
#include "ThemeUtils.h"
#include "taskdeadline.h"

MainWindowUI::MainWindowUI(QObject *parent) : QObject(parent) {
    taskTracker = new TaskStatusTracker();
    taskSearch = new TaskSearchHelper(this);
}

void MainWindowUI::setupUI(QMainWindow *mainWindow) {
    centralWidget = new QWidget(mainWindow);
    mainWindow->setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. Khởi tạo topBar và Layout ngang
    topBar = new QWidget(mainWindow);
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

    taskSearch->setupInWidget(topBar);
    topBarLayout->addWidget(taskSearch->searchButton);
    topBarLayout->addWidget(taskSearch->searchEdit);

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
    scrollArea = new QScrollArea(mainWindow);
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
    bottomBar = new QWidget(mainWindow);
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
    taskTracker->setupInWidget(bottomBar);
    bottomBarLayout->addWidget(taskTracker->statusLabel);
    bottomBarLayout->addWidget(taskTracker->progressBar);

    bottomBarLayout->addSpacing(15);

    bottomBarLayout->addWidget(undoButton);

    // Ghép Layout
    mainLayout->addWidget(topBar, 0);      
    mainLayout->addWidget(scrollArea, 1);
    mainLayout->addWidget(bottomBar, 0);
}

void MainWindowUI::clearLayout() {
    if (!contentLayout) return;

    QLayoutItem *child;
    while ((child = contentLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

void MainWindowUI::renderTaskItem(const Task &task, QObject *eventFilterObj) {
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
    taskItem->installEventFilter(eventFilterObj);

    contentLayout->addWidget(taskItem);
}

QWidget* MainWindowUI::createTextContainer(const Task &task, QWidget *parent) {
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

    // Sử dụng class bọc TaskDeadline để xử lý chuỗi và kiểm tra logic quá hạn
    TaskDeadline deadlineWrapper(task.getDeadline());

    QString dtStr = deadlineWrapper.toString("dd/MM/yyyy hh:mm");
    QLabel *deadlineLabel = new QLabel("Deadline: " + dtStr, textContainer);
    QString deadlineStyle = "font-size: 13px; background: transparent;";

    if (deadlineWrapper.isOverdue(task.statusToString(task.getStatus()))) {
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

QLabel* MainWindowUI::createDescriptionLabel(const QString &desc, QWidget *parent) {
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

QWidget* MainWindowUI::createStatusBadge(TaskStatus status, QWidget *parent) {
    QWidget *statusColorWidget = new QWidget(parent);
    QString statusColor = "#95a5a6";
    if (status == TaskStatus::TODO) statusColor = "#e74c3c";
    else if (status == TaskStatus::IN_PROGRESS) statusColor = "#f1c40f";
    else if (status == TaskStatus::DONE) statusColor = "#2ecc71";

    statusColorWidget->setStyleSheet(QString("background-color: %1; border-radius: 6px;").arg(statusColor));
    statusColorWidget->setFixedSize(12, 12);
    return statusColorWidget;
}

QToolButton* MainWindowUI::createDeleteButton(int taskId, QWidget *parent) {
    QToolButton *deleteBtn = new QToolButton(parent);
    deleteBtn->setText("🗑");
    deleteBtn->setStyleSheet(
        "QToolButton { color: #e74c3c; font-size: 18px; background: transparent; border: none; }"
        "QToolButton:hover { color: #c0392b; font-size: 20px; }"
        );
    deleteBtn->setProperty("taskId", taskId);
    connect(deleteBtn, &QToolButton::clicked, this, &MainWindowUI::onInternalDeleteClicked);
    return deleteBtn;
}

QCheckBox* MainWindowUI::createStatusCheckBox(const Task &task, QWidget *parent) {
    QCheckBox *checkBox = new QCheckBox(parent);
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
    connect(checkBox, &QCheckBox::stateChanged, this, &MainWindowUI::onInternalStatusChanged);
    return checkBox;
}

void MainWindowUI::onInternalDeleteClicked() {
    QToolButton *senderBtn = qobject_cast<QToolButton*>(sender());
    if (senderBtn) {
        emit deleteTaskClicked(senderBtn->property("taskId").toInt());
    }
}

void MainWindowUI::onInternalStatusChanged(int state) {
    QCheckBox *senderCb = qobject_cast<QCheckBox*>(sender());
    if (senderCb) {
        emit taskStatusChanged(senderCb->property("taskId").toInt(), state);
    }
}
