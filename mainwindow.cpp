#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDateTime>

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
    titleLabel = new QLabel("Task", topBar);
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
    mainLayout->addWidget(topBar, 1);      // Chiếm 20% hoặc tự động co theo nội dung topbar
    mainLayout->addWidget(scrollArea, 7);  // Chiếm toàn bộ phần còn lại và có thể cuộn

    // ====================================================
    // 💡 VÍ DỤ MINH HỌA: Thêm thử 20 cái nhãn để text dài ra xem nó cuộn
    // ====================================================
    connect(newButton, &QPushButton::clicked, this, &MainWindow::addNewTask);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addNewTask()
{
    // 1. Tạo một Widget tổng làm khung bọc cho cả 1 dòng Task
    QWidget *taskItem = new QWidget(scrollContent);
    taskItem->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border-radius: 8px;"
        "   min-height: 70px;"
        "}"
        );

    taskItem->setFixedHeight(70);

    // 2. Tạo Layout ngang cho taskItem (để chia trái: Chữ, phải: Ô check)
    QHBoxLayout *taskLayout = new QHBoxLayout(taskItem);
    taskLayout->setContentsMargins(15, 10, 15, 10);

    // 3. Tạo một Widget phụ và Layout dọc bên trái để chứa Tên & Deadline xếp chồng lên nhau
    QWidget *textContainer = new QWidget(taskItem);
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4); // Khoảng cách giữa tên và deadline

    // Nhãn tên mặc định "Unidentified" ở góc trên trái
    QLabel *nameLabel = new QLabel("Unidentified", textContainer);
    nameLabel->setStyleSheet("color: #2c3e50; font-size: 15px; font-weight: bold; background: transparent;");

    // Nhãn deadline ở dưới (lấy tạm thời gian hiện tại + 1 ngày làm mẫu)
    QString currentDateTime = QDateTime::currentDateTime().addDays(1).toString("dd/MM/yyyy hh:mm");
    QLabel *deadlineLabel = new QLabel("Deadline: " + currentDateTime, textContainer);
    deadlineLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; background: transparent;");

    // Thêm tên và deadline vào layout dọc bên trái
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(deadlineLabel);

    // 4. Tạo ô check (QCheckBox) nằm ở bên phải
    QCheckBox *checkBox = new QCheckBox(taskItem);
    checkBox->setStyleSheet(
        "QCheckBox::indicator {"
        "   width: 20px;"
        "   height: 20px;"
        "}"
        );

    // 5. Lắp ráp các thành phần vào Layout ngang của Task
    taskLayout->addWidget(textContainer); // Bên trái chứa cụm text
    taskLayout->addStretch();             // Khoảng trống ở giữa đẩy checkbox sang phải
    taskLayout->addWidget(checkBox);      // Bên phải chứa checkbox

    // 6. Đưa toàn bộ viên Task vừa tạo vào danh sách cuộn chính
    contentLayout->addWidget(taskItem);
}