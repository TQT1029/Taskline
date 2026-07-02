#ifndef TASKSEARCH_H
#define TASKSEARCH_H

#include <QLineEdit>
#include <QToolButton>
#include <QCompleter>
#include <QTimer>
#include <QStringListModel>
#include <QList>
#include <QAbstractItemView>
#include <QFocusEvent>
#include <QPropertyAnimation>
#include "task.h"

class CustomSearchEdit : public QLineEdit {
    Q_OBJECT
private:
    QToolButton *m_btn;
    QCompleter *m_completer;
public:
    CustomSearchEdit(QWidget *parent, QToolButton *btn, QCompleter *completer);

protected:
    void focusOutEvent(QFocusEvent *event) override;
};

class TaskSearchHelper : public QObject {
    Q_OBJECT
public:
    QToolButton *searchButton = nullptr;
    QLineEdit *searchEdit = nullptr;
    QCompleter *searchCompleter = nullptr;
    QStringListModel *completerModel = nullptr;
    QTimer *searchTimer = nullptr;

    explicit TaskSearchHelper(QObject *parent = nullptr);

    void setupInWidget(QWidget *parentBar);
    void updateSuggestions(const QList<Task> &allTasks);

signals:
    void searchRequested();
};

#endif // TASKSEARCH_H