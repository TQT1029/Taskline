#ifndef TASKNOTIFIER_H
#define TASKNOTIFIER_H

#include <QSystemTrayIcon>
#include <QIcon>
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QSet>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include "task.h"

class TaskNotifier : public QObject {
    Q_OBJECT
private:
    QSystemTrayIcon *trayIcon = nullptr;
    QTimer *checkTimer = nullptr;
    QMenu *trayMenu = nullptr;
    QAction *closeAction = nullptr;

    QSet<QString> notifiedOverdueTasks;
    QSet<QString> notifiedUrgentTasks;
    QSet<QString> notifiedOneHourWarningTasks;

public:
    explicit TaskNotifier(QObject *parent = nullptr);
    ~TaskNotifier();

    void sendNotification(const QString &title, const QString &message,
                          QSystemTrayIcon::MessageIcon iconType = QSystemTrayIcon::Information,
                          int durationMs = 3000);

    void checkDeadlines(const QList<Task> &allTasks);
    void clearTaskCache(const QString &taskId);
};

#endif // TASKNOTIFIER_H