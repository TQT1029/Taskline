#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include "taskdialog.h"

MainWindow::MainWindow(QWidget *parent)

    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(" ");
    if (ui->menubar) {
        ui->menubar->hide();
    }

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0 ,0 ,0 ,0);
    mainLayout->setSpacing(0);

    // 1. Khởi tạo topBar và Layout ngang
    topBar = new QWidget(this);
    topBar->setStyleSheet("background-color: #2c3e50;");

    topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(15, 0, 15, 0); // Cách lề trái/phải 15px cho đẹp
    topBarLayout->setSpacing(15);                  // Khoảng cách giữa nút 3 gạch và chữ Task

    // 2. Khởi tạo và định dạng nút 3 gạch (Menu Button)
    menuButton = new QToolButton(topBar);
    menuButton->setText("≡"); // Ký tự 3 gạch (Hoặc dùng "☰")
    menuButton->setStyleSheet(
        "QToolButton {"
        "   color: white;"
        "   font-size: 40px;"            // Làm cho dấu 3 gạch to lên
        "   background: transparent;"    // Nền trong suốt
        "   border: none;"
        "   padding-bottom: 4px;"        // Căn chỉnh nhẹ vị trí ký tự
        "}"
        "QToolButton:hover {"
        "   color: #3498db;"             // Đổi sang màu xanh khi rê chuột vào
        "}"
        );

    // 3. Khởi tạo và định dạng nhãn chữ "Task"
    titleLabel = new QLabel("Taskline", topBar);
    titleLabel->setStyleSheet(
        "color: white;"
        "font-size: 22px;"
        "font-weight: bold;"
        "font-family: 'Segoe UI', sans-serif;"
        "border: none;"
        );

    // 4. Thêm các thành phần vào Layout theo thứ tự từ trái sang phải
    topBarLayout->addWidget(menuButton); // Nút 3 gạch nằm ngoài cùng bên trái
    topBarLayout->addWidget(titleLabel); // Chữ Task nằm kế bên

    // 5. Thêm một khoảng trống lò xo (Spacer) ở cuối
    // Lò xo này sẽ tự giãn ra và ĐẨY nút + chữ về sát cạnh trái
    topBarLayout->addStretch();

    newButton = new QPushButton("+ New", topBar);
    newButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"  // Màu nền xanh dương
        "   color: white;"               // Màu chữ trắng
        "   border-radius: 13px;"        // ⭐ Bo góc chuẩn 13px theo yêu cầu
        "   font-weight: bold;"          // Chữ in đậm
        "   font-size: 15px;"            // ⭐ Tăng cỡ chữ lên một chút cho cân đối
        "   border: none;"               // Xóa viền mặc định

        // ⭐ BÍ QUYẾT LÀM NÚT TO RA VÀ CHỮ Ở GIỮA:
        "   min-width: 60px;"           // ⭐ Chiều rộng tối thiểu của nút
        "   min-height: 38px;"           // ⭐ Chiều cao tối thiểu (làm nút to ra rõ rệt)
        "   padding: 0px 20px 0px 20px;"          // ⭐ Đệm đều 2 bên, chữ tự động khóa ở tâm nút
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"  // Màu khi hover
        "}"
        "QPushButton:pressed {"
        "   background-color: #1c5980;"  // Màu khi click
        "}"
        );

    topBarLayout->addWidget(newButton);


    scrollArea = new QScrollArea(ui->centralwidget);
    scrollArea->setWidgetResizable(true); // ⭐ QUAN TRỌNG: Cho phép nội dung tự co giãn theo chiều rộng
    scrollArea->setStyleSheet("background-color: #ecf0f1;"); // Màu nền vùng cuộn

    // Tùy chỉnh thanh cuộn nhìn cho thanh lịch, hiện đại hơn (QSS)
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

    // 2. Khởi tạo Widget "Ruột" để chứa tất cả nội dung dài sau này
    scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;"); // Để lộ màu nền của scrollArea

    // 3. Tạo Layout cho Widget "Ruột" này để xếp nội dung theo chiều dọc
    contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(20, 20, 20, 20); // Tạo khoảng đệm xung quanh nội dung
    contentLayout->setSpacing(15);                     // Khoảng cách giữa các item bên trong

    contentLayout->setAlignment(Qt::AlignTop);
    // 4. Gắn cái "Ruột" vào khung cuộn QScrollArea
    scrollArea->setWidget(scrollContent);

    // 5. Thêm topBar (Cố định) và scrollArea (Cuộn được) vào layout chính của màn hình
    mainLayout->addWidget(topBar, 0);      
    mainLayout->addWidget(scrollArea, 1);

    // 6. Khởi tạo bottomBar chứa nút Undo
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
    
    mainLayout->addWidget(bottomBar, 0);

    // Tải dữ liệu từ TaskManager
    dataFilePath = QDir::currentPath() + "/tasks.json";
    taskManager.loadFromFile(dataFilePath);
    
    connect(newButton, &QPushButton::clicked, this, &MainWindow::addNewTask);
    connect(undoButton, &QPushButton::clicked, this, &MainWindow::onUndoTaskClicked);

    // Hiển thị danh sách task ban đầu
    refreshTaskList();
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
        refreshTaskList();
    }
}

void MainWindow::refreshTaskList()
{
    // Xóa tất cả các widget con trong contentLayout
    QLayoutItem *child;
    while ((child = contentLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    QList<Task> tasks = taskManager.getAllTasks();
    for (const Task &task : tasks) {
        QWidget *taskItem = new QWidget(scrollContent);
        taskItem->setObjectName("taskItem");
        taskItem->setStyleSheet(
            "QWidget#taskItem {"
            "   background-color: white;"
            "   border-radius: 10px;"
            "   border: 1px solid #dcdde1;"
            "   min-height: 75px;"
            "}"
            "QWidget#taskItem:hover {"
            "   border: 1px solid #3498db;"
            "   background-color: #f8fbfc;"
            "}"
        );
        taskItem->setFixedHeight(75);

        QHBoxLayout *taskLayout = new QHBoxLayout(taskItem);
        taskLayout->setContentsMargins(15, 10, 15, 10);

        QWidget *textContainer = new QWidget(taskItem);
        QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(4);

        QLabel *nameLabel = new QLabel(task.getTitle(), textContainer);
        QString textStyle = "color: #2c3e50; font-size: 15px; font-weight: bold; background: transparent;";
        if (task.getStatus() == TaskStatus::DONE) {
            textStyle += " text-decoration: line-through; color: #95a5a6;";
        }
        nameLabel->setStyleSheet(textStyle);

        QString dtStr = task.getDeadline().toString("dd/MM/yyyy hh:mm");
        QLabel *deadlineLabel = new QLabel("Deadline: " + dtStr, textContainer);
        
        QString deadlineStyle = "font-size: 12px; background: transparent;";
        if (task.isOverdue()) {
            deadlineStyle += " color: #e74c3c; font-weight: bold;";
        } else {
            deadlineStyle += " color: #7f8c8d;";
        }
        deadlineLabel->setStyleSheet(deadlineStyle);

        textLayout->addWidget(nameLabel);
        textLayout->addWidget(deadlineLabel);

        QCheckBox *checkBox = new QCheckBox(taskItem);
        checkBox->setStyleSheet(
            "QCheckBox::indicator {"
            "   width: 20px;"
            "   height: 20px;"
            "}"
        );
        checkBox->setChecked(task.getStatus() == TaskStatus::DONE);
        checkBox->setProperty("taskId", task.getId());
        
        connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::onTaskStatusChanged);

        QWidget *statusColorWidget = new QWidget(taskItem);

        // Xác định màu sắc dựa trên trạng thái của Task
        QString statusColor = "#95a5a6"; // Mặc định (Xám)
        if (task.getStatus() == TaskStatus::TODO) {
            statusColor = "#e74c3c";     // Chưa làm: Đỏ nhạt
        } else if (task.getStatus() == TaskStatus::IN_PROGRESS) {
            statusColor = "#f1c40f";     // Đang làm: Vàng
        } else if (task.getStatus() == TaskStatus::DONE) {
            statusColor = "#2ecc71";     // Hoàn thành: Xanh lá
        }

        // Định dạng Widget thành một hình tròn nhỏ (hoặc hình chữ nhật bo góc)
        statusColorWidget->setStyleSheet(
            QString("background-color: %1; border-radius: 6px;").arg(statusColor)
            );
        statusColorWidget->setFixedSize(12, 12);

        QToolButton *deleteBtn = new QToolButton(taskItem);
        deleteBtn->setText("🗑");
        deleteBtn->setStyleSheet(
            "QToolButton {"
            "   color: #e74c3c; font-size: 18px; background: transparent; border: none;"
            "}"
            "QToolButton:hover {"
            "   color: #c0392b; font-size: 20px;"
            "}"
        );
        deleteBtn->setProperty("taskId", task.getId());
        connect(deleteBtn, &QToolButton::clicked, this, &MainWindow::onDeleteTaskClicked);

        taskLayout->addWidget(textContainer);
        taskLayout->addStretch();
        taskLayout->addWidget(statusColorWidget);
        taskLayout->addWidget(deleteBtn);
        taskLayout->addWidget(checkBox);

        // Bắt sự kiện click vào task để mở Details
        taskItem->setProperty("taskId", task.getId());
        taskItem->setCursor(Qt::PointingHandCursor);
        taskItem->installEventFilter(this);

        contentLayout->addWidget(taskItem);
    }
}

void MainWindow::onTaskStatusChanged(int state)
{
    QCheckBox *senderCb = qobject_cast<QCheckBox*>(sender());
    if (senderCb) {
        int taskId = senderCb->property("taskId").toInt();
        QList<Task> allTasks = taskManager.getAllTasks();
        Task targetTask = allTasks.first(); // Fallback
        
        for (const Task &t : allTasks) {
            if (t.getId() == taskId) {
                targetTask = t;
                break;
            }
        }
        
        if (state == Qt::Checked) {
            taskManager.markTaskDone(taskId);
        } else {
            taskManager.editTask(taskId, targetTask.getTitle(), targetTask.getDescription(), TaskStatus::TODO, targetTask.getPriority(), targetTask.getDeadline());
        }
        taskManager.saveToFile(dataFilePath);
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
        refreshTaskList();
    }
}

void MainWindow::onUndoTaskClicked()
{
    taskManager.undoDelete();
    taskManager.saveToFile(dataFilePath);
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