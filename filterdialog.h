#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QHBoxLayout>

class FilterDialog : public QDialog {
    Q_OBJECT
public:
    explicit FilterDialog(int initStatus, int initPriority, int initSort, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Bộ lọc & Sắp xếp");
        setMinimumWidth(300);

        QFormLayout *layout = new QFormLayout(this);

        // 1. Lọc theo trạng thái
        statusCombo = new QComboBox(this);
        statusCombo->addItems({"Tất cả", "Chưa làm (TODO)", "Đang làm (IN_PROGRESS)", "Hoàn thành (DONE)"});
        statusCombo->setCurrentIndex(initStatus);
        layout->addRow("Trạng thái:", statusCombo);

        // 2. Lọc theo độ ưu tiên (Vì trong Task.h priority là kiểu int nên dùng QSpinBox)
        // Giá trị 0 quy ước là "Tất cả" không lọc
        priorityInput = new QSpinBox(this);
        priorityInput->setRange(0, 100);
        priorityInput->setSpecialValueText("Tất cả (0)");
        priorityInput->setValue(initPriority);
        layout->addRow("Độ ưu tiên:", priorityInput);

        // 3. Sắp xếp theo Deadline
        sortCombo = new QComboBox(this);
        sortCombo->addItems({"Mặc định", "Deadline: Sắp tới trước", "Deadline: Xa nhất trước"});
        sortCombo->setCurrentIndex(initSort);
        layout->addRow("Sắp xếp hạn:", sortCombo);

        // Nút bấm xác nhận / hủy
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *okBtn = new QPushButton("Áp dụng", this);
        QPushButton *cancelBtn = new QPushButton("Hủy", this);
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(cancelBtn);
        layout->addRow(btnLayout);

        connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    }

    int getSelectedStatus() const { return statusCombo->currentIndex(); }
    int getSelectedPriority() const { return priorityInput->value(); }
    int getSortOrder() const { return sortCombo->currentIndex(); }

private:
    QComboBox *statusCombo;
    QSpinBox *priorityInput;
    QComboBox *sortCombo;
};

#endif // FILTERDIALOG_H