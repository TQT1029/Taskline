#include "taskdialog.h"
#include <QFormLayout>
#include <QMessageBox>
#include "ThemeUtils.h"

TaskDialog::TaskDialog(QWidget *parent) : QDialog(parent), m_editable(true)
{
    setWindowTitle("Task Details");
    setMinimumWidth(400);

    // Style tổng thể (Giữ nguyên cấu trúc CSS của bạn)
    setStyleSheet(QString(
                      "QDialog { background-color: %1; }"
                      "QLabel { color: %2; font-size: 14px; font-weight: bold; font-family: 'Segoe UI', sans-serif; }"
                      "QLineEdit, QTextEdit, QComboBox, QSpinBox, QDateTimeEdit {"
                      "   border: 1px solid %3; border-radius: 4px; padding: 5px; background: %4; color: %5;"
                      "}")
                      .arg(ThemeUtils::dialogBg(), ThemeUtils::textMain(), ThemeUtils::inputBorder(), ThemeUtils::inputBg(), ThemeUtils::textMain())
                  + QString(
                        "QSpinBox::up-button, QDateTimeEdit::up-button {"
                        "    subcontrol-origin: border;"
                        "    subcontrol-position: top right;"
                        "    width: 25px;"
                        "   border-top-right-radius: 4px;"
                        "   border-left: 1px solid %1;"
                        "   border-bottom: 1px solid %1;"
                        "   background: %2;"
                        "}"
                        "QSpinBox::up-button:hover, QDateTimeEdit::up-button:hover { background: %3; }"
                        "QSpinBox::up-button:disabled, QDateTimeEdit::up-button:disabled { background: %4; border-left: none; }"

                        "QSpinBox::down-button, QDateTimeEdit::down-button {"
                        "    subcontrol-origin: border;"
                        "    subcontrol-position: bottom right;"
                        "   width: 25px;"
                        "   border-bottom-right-radius: 4px;"
                        "   border-left: 1px solid %1;"
                        "   background: %2;"
                        "}"
                        "QSpinBox::down-button:hover, QDateTimeEdit::down-button:hover { background: %3; }"
                        "QSpinBox::down-button:disabled, QDateTimeEdit::down-button:disabled { background: %4; border-left: none; }"
                        ).arg(ThemeUtils::inputBorder(), ThemeUtils::spinBtnBg(), ThemeUtils::spinBtnHover(), ThemeUtils::inputDisabledBg())
                  + QString(
                        "QSpinBox, QDateTimeEdit {"
                        "    padding-right: 30px;"
                        "}"

                        "QLineEdit:disabled, QTextEdit:disabled, QComboBox:disabled, QSpinBox:disabled, QDateTimeEdit:disabled {"
                        "   background: %1; color: %2;"
                        "}"

                        "QComboBox QAbstractItemView {"
                        "   background-color: %3;"
                        "   color: %4;"
                        "   selection-background-color: %5;"
                        "}"

                        "QPushButton#btnLockToggle, QPushButton#btnCancel {"
                        "   background-color: %6; color: %7; font-weight: bold; border-radius: 6px; padding: 6px 15px; border: none;"
                        "}"
                        "QPushButton#btnLockToggle:hover, QPushButton#btnCancel:hover { background-color: %8; }"
                        "QPushButton {"
                        "   background-color: %9; color: %10; font-weight: bold; border-radius: 6px; padding: 6px 15px; border: none;"
                        "}"
                        "QPushButton:hover { background-color: %11; }"
                        ).arg(
                            ThemeUtils::inputDisabledBg(), ThemeUtils::inputDisabledText(),
                            ThemeUtils::comboDropBg(), ThemeUtils::textMain(), ThemeUtils::btnPrimary(),
                            ThemeUtils::btnSecondary(), ThemeUtils::btnSecondaryText(), ThemeUtils::btnSecondaryHover(),
                            ThemeUtils::btnPrimary(), ThemeUtils::btnPrimaryText(), ThemeUtils::btnPrimaryHover()
                            ));

    // Khởi tạo các ô nhập liệu
    titleInput = new QLineEdit(this);
    descInput = new QTextEdit(this);
    descInput->setMinimumHeight(80);

    // THAY ĐỔI: Khởi tạo nhãn QLabel tĩnh hiển thị trạng thái mặc định ban đầu là "Đang làm"
    statusLabel = new QLabel("Đang làm", this);
    statusLabel->setStyleSheet(QString("font-size: 14px; color: %1; font-weight: bold; padding: 5px;").arg(ThemeUtils::textMain()));

    priorityInput = new QSpinBox(this);
    priorityInput->setRange(1, 10);
    priorityInput->setValue(1);

    deadlineInput = new DeadlineSelectorWidget(this);

    btnLockToggle = new QPushButton("Mở khóa (Unlock)", this);
    btnLockToggle->setObjectName("btnLockToggle");
    btnSave = new QPushButton("Save", this);
    btnCancel = new QPushButton("Cancel", this);
    btnCancel->setObjectName("btnCancel");

    // Bố cục Form
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(btnLockToggle);
    mainLayout->addLayout(topLayout);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("Tiêu đề:", titleInput);
    formLayout->addRow("Mô tả:", descInput);
    formLayout->addRow("Trạng thái:", statusLabel); // Đưa nhãn trạng thái tĩnh vào đây
    formLayout->addRow("Ưu tiên (1-10):", priorityInput);
    formLayout->addRow("Hạn chót:", deadlineInput);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);

    connect(btnLockToggle, &QPushButton::clicked, this, &TaskDialog::toggleEditMode);
    connect(btnSave, &QPushButton::clicked, this, &TaskDialog::onSaveClicked);
    connect(btnCancel, &QPushButton::clicked, this, &TaskDialog::onCancelClicked);

    setEditMode(true);
}

void TaskDialog::setTaskData(const Task &task)
{
    titleInput->setText(task.getTitle());
    descInput->setText(task.getDescription());

    // THAY ĐỔI: Hiển thị trạng thái tĩnh tương ứng của Task lên nhãn chữ
    if (task.getStatus() == TaskStatus::TODO) {
        statusLabel->setText("Chưa làm");
        statusLabel->setStyleSheet("font-size: 14px; color: #e74c3c; font-weight: bold;"); // Chữ đỏ cảnh báo
    } else if (task.getStatus() == TaskStatus::IN_PROGRESS) {
        statusLabel->setText("Đang làm");
        statusLabel->setStyleSheet("font-size: 14px; color: #f1c40f; font-weight: bold;"); // Chữ vàng đang làm
    } else if (task.getStatus() == TaskStatus::DONE) {
        statusLabel->setText("Hoàn thành");
        statusLabel->setStyleSheet("font-size: 14px; color: #2ecc71; font-weight: bold;"); // Chữ xanh hoàn thành
    }

    priorityInput->setValue(task.getPriority());
    deadlineInput->setDateTime(task.getDeadline());

    setEditMode(false);
}

void TaskDialog::setEditMode(bool editable)
{
    m_editable = editable;

    titleInput->setEnabled(editable);
    statusLabel->setEnabled(editable);
    priorityInput->setEnabled(editable);
    deadlineInput->setEnabled(editable);

    descInput->setEnabled(true);
    descInput->setReadOnly(!editable);

    if (editable) {
        descInput->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3;").arg(ThemeUtils::inputBg(), ThemeUtils::textMain(), ThemeUtils::inputBorder()));
    } else {
        descInput->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3;").arg(ThemeUtils::inputDisabledBg(), ThemeUtils::inputDisabledText(), ThemeUtils::inputBorder()));
    }

    if (editable) {
        btnLockToggle->setText("🔓 Khóa (Lock)");
        btnSave->setEnabled(true);
    } else {
        btnLockToggle->setText("🔒 Mở khóa (Unlock)");
        btnSave->setEnabled(false);
    }
}

bool TaskDialog::isEditable() const
{
    return m_editable;
}

void TaskDialog::toggleEditMode()
{
    setEditMode(!m_editable);
}

void TaskDialog::onSaveClicked()
{
    if (titleInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Tiêu đề không được để trống!");
        return;
    }
    accept();
}

void TaskDialog::onCancelClicked()
{
    reject();
}

QString TaskDialog::getTitle() const { return titleInput->text(); }
QString TaskDialog::getDescription() const { return descInput->toPlainText(); }
TaskStatus TaskDialog::getStatus() const
{
    QString currentText = statusLabel->text();
    if (currentText == "Chưa làm") return TaskStatus::TODO;
    if (currentText == "Hoàn thành") return TaskStatus::DONE;
    return TaskStatus::IN_PROGRESS; // Mặc định trả về Đang làm
}
int TaskDialog::getPriority() const { return priorityInput->value(); }
TaskDeadline TaskDialog::getDeadline() const
{
    return TaskDeadline(deadlineInput->getDateTime());
}