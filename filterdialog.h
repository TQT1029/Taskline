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
    explicit FilterDialog(int initStatus = 0, int initPriority = 0, int initSort = 0, QWidget *parent = nullptr);

    int getSelectedStatus() const;
    int getSelectedPriority() const;
    int getSortOrder() const;

private:
    QComboBox *statusCombo;
    QSpinBox *priorityInput;
    QComboBox *sortCombo;
};

#endif // FILTERDIALOG_H