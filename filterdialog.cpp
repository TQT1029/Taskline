#include "filterdialog.h"
#include "ThemeUtils.h"

FilterDialog::FilterDialog(int initStatus, int initPriority, int initSort, QWidget *parent)
    : QDialog(parent)
{   
    setWindowTitle("Bộ lọc & Sắp xếp");
    setMinimumWidth(340);
    setContentsMargins(20, 20, 20, 20);

    this->setStyleSheet(QString(
        // Nền của Dialog
        "QDialog { background-color: %1; }"

        // Định dạng chung cho Nhãn chữ (Labels)
        "QLabel { color: %2; font-size: 14px; font-weight: bold; font-family: 'Segoe UI', sans-serif; }"

        // Định dạng cho ô ComboBox và SpinBox
        "QComboBox, QSpinBox {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 8px;"
        "    padding: 6px 12px;"
        "    font-size: 14px;"
        "    color: %5;"
        "    min-height: 25px;"
        "}"
        "QComboBox:hover, QSpinBox:hover {"
        "    border: 1px solid %6;"
        "}"
        "QComboBox:focus, QSpinBox:focus {"
        "    border: 1px solid %6;"
        "}"

        // 1. Tùy chỉnh phần mũi tên của ComboBox (Cho đồng bộ)
        "QComboBox::drop-down {"
        "    border: none;"
        "    subcontrol-origin: padding;"
        "    subcontrol-position: top right;"
        "    width: 30px;"
        "}"

        "QSpinBox::up-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: top right;" // Đặt nút tăng ở góc trên bên phải
        "    width: 25px;"                     // Tăng độ rộng để hitbox to hơn
        "    border-top-right-radius: 8px;"    // Bo góc theo khung viền cha
        "    border-left: 1px solid %4;"
        "    border-bottom: 1px solid %4;"
        "    background: %7;"
        "}"
        "QSpinBox::up-button:hover { background: %8; }"

        "QSpinBox::down-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: bottom right;" // Đặt nút giảm ở góc dưới bên phải
        "    width: 25px;"                       // Tăng độ rộng bằng nút tăng
        "    border-bottom-right-radius: 8px;"   // Bo góc theo khung viền cha
        "    border-left: 1px solid %4;"
        "    background: %7;"
        "}"
        "QSpinBox::down-button:hover { background: %8; }"

        "QSpinBox {"
        "    padding-right: 30px;"
        "}"

        // Định dạng nút "Áp dụng" (Nút chính)
        "QPushButton#okBtn {"
        "    background-color: %9;"
        "    color: %10;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border-radius: 8px;"
        "    min-height: 35px;"
        "    border: none;"
        "}"
        "QPushButton#okBtn:hover { background-color: %11; }"

        // Định dạng nút "Hủy" (Nút phụ)
        "QPushButton#cancelBtn {"
        "    background-color: %12;"
        "    color: %13;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border-radius: 8px;"
        "    min-height: 35px;"
        "    border: none;"
        "}"
        "QPushButton#cancelBtn:hover { background-color: %14; }"
        "QComboBox QAbstractItemView {"
        "   background-color: %15;"
        "   color: %16;"
        "   selection-background-color: %9;"
        "}"
        ).arg(ThemeUtils::dialogBg(), ThemeUtils::textMain(), ThemeUtils::inputBg(), ThemeUtils::inputBorder(), ThemeUtils::textMain())
         .arg(ThemeUtils::inputFocus(), ThemeUtils::spinBtnBg(), ThemeUtils::spinBtnHover(), ThemeUtils::btnPrimary(), ThemeUtils::btnPrimaryText())
         .arg(ThemeUtils::btnPrimaryHover(), ThemeUtils::btnSecondary(), ThemeUtils::btnSecondaryText(), ThemeUtils::btnSecondaryHover())
         .arg(ThemeUtils::comboDropBg(), ThemeUtils::textMain())
    );

    QFormLayout *layout = new QFormLayout(this);
    layout->setSpacing(15);

    // 1. Lọc theo trạng thái
    statusCombo = new QComboBox(this);
    statusCombo->addItems({"Tất cả", "Chưa làm (TODO)", "Đang làm (IN_PROGRESS)", "Hoàn thành (DONE)"});
    statusCombo->setCurrentIndex(initStatus);
    layout->addRow("Trạng thái:", statusCombo);

    // 2. Lọc theo độ ưu tiên (Vì trong Task.h priority là kiểu int nên dùng QSpinBox)
    // Giá trị 0 quy ước là "Tất cả" không lọc
    priorityInput = new QSpinBox(this);
    priorityInput->setRange(0, 10);
    priorityInput->setSpecialValueText("Tất cả (0)");
    priorityInput->setValue(initPriority);
    layout->addRow("Độ ưu tiên:", priorityInput);

    // 3. Sắp xếp theo Deadline
    sortCombo = new QComboBox(this);
    sortCombo->addItems({"Mặc định", "Deadline: Sắp tới trước", "Deadline: Xa nhất trước"});
    sortCombo->setCurrentIndex(initSort);
    layout->addRow("Sắp xếp hạn:", sortCombo);

    layout->addItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Nút bấm xác nhận / hủy
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10); // Khoảng cách giữa 2 nút

    QPushButton *okBtn = new QPushButton("Áp dụng", this);
    okBtn->setObjectName("okBtn"); // Đặt tên ID để nhận đúng CSS riêng

    QPushButton *cancelBtn = new QPushButton("Hủy", this);
    cancelBtn->setObjectName("cancelBtn");

    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addRow(btnLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

int FilterDialog::getSelectedStatus() const { return statusCombo->currentIndex(); }
int FilterDialog::getSelectedPriority() const { return priorityInput->value(); }
int FilterDialog::getSortOrder() const { return sortCombo->currentIndex(); }
