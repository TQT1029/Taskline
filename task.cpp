#include "task.h"

// Sử dụng Member Initializer List để khởi tạo giá trị cho các biến
Task::Task(int id, const QString &title, TaskStatus status, int priority, const QDateTime &deadline)
    : id(id), title(title), status(status), priority(priority), deadline(deadline) {}

int Task::getId() const {
    return id;
}

QString Task::getTitle() const {
    return title;
}

TaskStatus Task::getStatus() const {
    return status;
}

int Task::getPriority() const {
    return priority;
}

QDateTime Task::getDeadline() const {
    return deadline;
}

void Task::setStatus(TaskStatus newStatus) {
    status = newStatus;
}

bool Task::isOverdue() const {
    // So sánh thời gian hiện tại với deadline và check xem đã hoàn thành chưa
    return (QDateTime::currentDateTime() > deadline && status != TaskStatus::DONE);
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