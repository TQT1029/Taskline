#ifndef TASK_H
#define TASK_H

#include <QDateTime>
#include <QString>
#include "TaskStats.h"

class Task
{
private:
    TaskStats taskStats;

public:
    // Constructor
    Task(int id,
         const QString &title,
         const QString &description,
         TaskStatus status,
         int priority,
         const QDateTime &deadline);

    // Các hàm Getters
    int getId() const;
    QString getTitle() const;
    QString getDescription() const;
    TaskStatus getStatus() const;
    int getPriority() const;
    QDateTime getDeadline() const;

    // Các hàm Setters / Logic khác
    void setTitle(QString title);
    void setDescription(QString description);
    void setStatus(TaskStatus newStatus);
    bool isOverdue() const; // Kiểm tra task quá hạn

    // Tiện ích chuyển đổi Enum và QString
    static QString statusToString(TaskStatus status);
    static TaskStatus stringToStatus(const QString &statusStr);
};

#endif // TASK_H
