#include "task.h"

// Sử dụng Member Initializer List để khởi tạo giá trị cho các biến
Task::Task(int id, const QString &title, const QString &description, TaskStatus status, int priority, const QDateTime &deadline){
    taskStats.id = id;
    taskStats.title = title;
    taskStats.description = description;
    taskStats.status = status;
    taskStats.priority = priority;
    taskStats.deadline = deadline;
}
// Getter
int Task::getId() const {
    return taskStats.id;
}

QString Task::getTitle() const {
    return taskStats.title;
}

QString Task::getDescription() const {
    return taskStats.description;
}

TaskStatus Task::getStatus() const {
    return taskStats.status;
}

int Task::getPriority() const {
    return taskStats.priority;
}

QDateTime Task::getDeadline() const {
    return taskStats.deadline;
}
// Setter
void Task::setId(int newId) {
    taskStats.id = newId;
}

void Task::setTitle(QString newTitle) {
    taskStats.title = newTitle;
}

void Task::setDescription(QString newDescription) {
    taskStats.description = newDescription;
}

void Task::setStatus(TaskStatus newStatus) {
    taskStats.status = newStatus;
}

bool Task::isOverdue() const {
    // So sánh thời gian hiện tại với deadline và check xem đã hoàn thành chưa
    return (QDateTime::currentDateTime() > taskStats.deadline && taskStats.status != TaskStatus::DONE);
}

QString Task::statusToString(TaskStatus status) {
    switch (status) {
    case TaskStatus::TODO: return "chưa làm";
    case TaskStatus::IN_PROGRESS: return "đang làm";
    case TaskStatus::DONE: return "hoàn thành";
    default: return "không xác định";
    }
}

TaskStatus Task::stringToStatus(const QString &statusStr) {
    if (statusStr == "đang làm") return TaskStatus::IN_PROGRESS;
    if (statusStr == "hoàn thành") return TaskStatus::DONE;
    return TaskStatus::TODO; // mặc định là chưa làm
}