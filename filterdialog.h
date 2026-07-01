#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include "ThemeUtils.h"

class FilterDialog : public QDialog {
    Q_OBJECT
public:
    explicit FilterDialog(int initStatus, int initPriority, int initSort, QWidget *parent = nullptr);

    int getSelectedStatus() const;
    int getSelectedPriority() const;
    int getSortOrder() const;

private:
    QComboBox *statusCombo;
    QSpinBox *priorityInput;
    QComboBox *sortCombo;
};

#endif // FILTERDIALOG_H