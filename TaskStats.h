#ifndef TASKSTATS_H
#define TASKSTATS_H
#include <qstring.h>
#include <QDateTime>

enum class TaskStatus {
    TODO,
    IN_PROGRESS,
    DONE
};

struct TaskStats{
public:
    QString id;
    QString title;
    QString description;
    TaskStatus status;
    int priority;   // Độ ưu tiên
    QDateTime deadline;
};
#endif // TASKSTATS_H
