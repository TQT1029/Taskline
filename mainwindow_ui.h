#ifndef MAINWINDOW_UI_H
#define MAINWINDOW_UI_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QCheckBox>
#include "task.h"
#include "taskstatusbar.h"
#include "tasksearch.h"

class MainWindowUI : public QObject {
    Q_OBJECT
public:
    explicit MainWindowUI(QObject *parent = nullptr);

    void setupUI(QMainWindow *mainWindow);
    void clearLayout();
    void renderTaskItem(const Task &task, QObject *eventFilterObj);

    // Publicly accessible widgets for the controller to connect signals
    QToolButton *menuButton = nullptr;
    QPushButton *newButton = nullptr;
    QPushButton *undoButton = nullptr;
    TaskStatusTracker *taskTracker = nullptr;
    TaskSearchHelper *taskSearch = nullptr;
    
signals:
    void deleteTaskClicked(QString taskId);
    void taskStatusChanged(QString taskId, int state);

private:
    QWidget* createTextContainer(const Task &task, QWidget *parent);
    QLabel* createDescriptionLabel(const QString &desc, QWidget *parent);
    QWidget* createStatusBadge(TaskStatus status, QWidget *parent);
    QToolButton* createDeleteButton(QString taskId, QWidget *parent);
    QCheckBox* createStatusCheckBox(const Task &task, QWidget *parent);

    // Layout elements
    QWidget *centralWidget = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    QWidget *topBar = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *scrollContent = nullptr;
    QVBoxLayout *contentLayout = nullptr;
    QLabel *titleLabel = nullptr;
    QHBoxLayout *topBarLayout = nullptr;
    QWidget *bottomBar = nullptr;
    QHBoxLayout *bottomBarLayout = nullptr;

private slots:
    void onInternalDeleteClicked();
    void onInternalStatusChanged(int state);
};

#endif // MAINWINDOW_UI_H
