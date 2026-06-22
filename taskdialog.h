#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "task.h"

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget *parent = nullptr);

    // Cài đặt dữ liệu cho View/Edit mode
    void setTaskData(const Task &task);
    
    // Thiết lập khóa/mở khóa nhập liệu
    void setEditMode(bool editable);
    bool isEditable() const;

    QString getTitle() const;
    QString getDescription() const;
    TaskStatus getStatus() const;
    int getPriority() const;
    QDateTime getDeadline() const;

private slots:
    void toggleEditMode();
    void onSaveClicked();
    void onCancelClicked();

private:
    QLineEdit *titleInput;
    QTextEdit *descInput;
    QComboBox *statusInput;
    QSpinBox *priorityInput;
    QDateTimeEdit *deadlineInput;

    QPushButton *btnLockToggle;
    QPushButton *btnSave;
    QPushButton *btnCancel;

    bool m_editable;
};

#endif // TASKDIALOG_H
