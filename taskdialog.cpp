#include "taskdialog.h"
#include <QFormLayout>
#include <QMessageBox>

TaskDialog::TaskDialog(QWidget *parent) : QDialog(parent), m_editable(true)
{
    setWindowTitle("Task Details");
    setMinimumWidth(400);

    // Style
    setStyleSheet(
        "QDialog { background-color: #f7f9fa; }"
        "QLabel { font-weight: bold; color: #2c3e50; }"
        "QLineEdit, QTextEdit, QComboBox, QSpinBox, QDateTimeEdit {"
        "   border: 1px solid #bdc3c7; border-radius: 4px; padding: 5px; background: white;"
        "}"
        "QLineEdit:disabled, QTextEdit:disabled, QComboBox:disabled, QSpinBox:disabled, QDateTimeEdit:disabled {"
        "   background: #ecf0f1; color: #7f8c8d;"
        "}"
        "QPushButton { background-color: #3498db; color: white; border-radius: 4px; padding: 8px 15px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton#btnCancel { background-color: #e74c3c; }"
        "QPushButton#btnCancel:hover { background-color: #c0392b; }"
        "QPushButton#btnLockToggle { background-color: #f39c12; }"
        "QPushButton#btnLockToggle:hover { background-color: #d68910; }"
    );

    // Khởi tạo controls
    titleInput = new QLineEdit(this);
    descInput = new QTextEdit(this);
    descInput->setMinimumHeight(80);

    statusInput = new QComboBox(this);
    statusInput->addItem("Chưa làm", QVariant((int)TaskStatus::TODO));
    statusInput->addItem("Đang làm", QVariant((int)TaskStatus::IN_PROGRESS));
    statusInput->addItem("Hoàn thành", QVariant((int)TaskStatus::DONE));

    priorityInput = new QSpinBox(this);
    priorityInput->setRange(1, 10);
    priorityInput->setValue(1);

    deadlineInput = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), this);
    deadlineInput->setCalendarPopup(true);

    btnLockToggle = new QPushButton("Mở khóa (Unlock)", this);
    btnLockToggle->setObjectName("btnLockToggle");
    btnSave = new QPushButton("Save", this);
    btnCancel = new QPushButton("Cancel", this);
    btnCancel->setObjectName("btnCancel");

    // Bố cục
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Nút lock ở trên cùng phải
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(btnLockToggle);
    mainLayout->addLayout(topLayout);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("Tiêu đề:", titleInput);
    formLayout->addRow("Mô tả:", descInput);
    formLayout->addRow("Trạng thái:", statusInput);
    formLayout->addRow("Ưu tiên (1-10):", priorityInput);
    formLayout->addRow("Thời hạn:", deadlineInput);
    
    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);

    // Kết nối
    connect(btnLockToggle, &QPushButton::clicked, this, &TaskDialog::toggleEditMode);
    connect(btnSave, &QPushButton::clicked, this, &TaskDialog::onSaveClicked);
    connect(btnCancel, &QPushButton::clicked, this, &TaskDialog::onCancelClicked);

    // Mặc định lúc tạo mới là có thể nhập
    setEditMode(true);
}

void TaskDialog::setTaskData(const Task &task)
{
    titleInput->setText(task.getTitle());
    descInput->setText(task.getDescription());
    
    int index = statusInput->findData(QVariant((int)task.getStatus()));
    if (index != -1) statusInput->setCurrentIndex(index);
    
    priorityInput->setValue(task.getPriority());
    deadlineInput->setDateTime(task.getDeadline());

    // Nếu đã có data thì mặc định là View Mode (bị khóa)
    setEditMode(false);
}

void TaskDialog::setEditMode(bool editable)
{
    m_editable = editable;
    titleInput->setEnabled(editable);
    descInput->setEnabled(editable);
    statusInput->setEnabled(editable);
    priorityInput->setEnabled(editable);
    deadlineInput->setEnabled(editable);

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
TaskStatus TaskDialog::getStatus() const { return (TaskStatus)statusInput->currentData().toInt(); }
int TaskDialog::getPriority() const { return priorityInput->value(); }
QDateTime TaskDialog::getDeadline() const { return deadlineInput->dateTime(); }
