#include "taskmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include <algorithm>

TaskManager::TaskManager() {}

void TaskManager::addTask(const QString &title, const QString &description, TaskStatus status, int priority, const QDateTime &deadline) {
    static int tempIdCounter = -1;
    int id = tempIdCounter--;

    tasks.append(Task(id, title, description, status, priority, deadline));
}

bool TaskManager::editTask(int id, const QString &title, const QString &description, TaskStatus status, int priority, const QDateTime &deadline) {
    for (auto &task : tasks) {
        if (task.getId() == id) {
            task = Task(id, title, description, status, priority, deadline);
            return true;
        }
    }
    return false;
}

bool TaskManager::updateTaskId(int oldId, int newId) {
    for (auto &task : tasks) {
        if (task.getId() == oldId) {
            task.setId(newId);
            return true;
        }
    }
    return false;
}

bool TaskManager::deleteTask(int id) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].getId() == id) {
            undoStack.push(tasks[i]);
            tasks.removeAt(i);
            return true;
        }
    }
    return false;
}

bool TaskManager::markTaskDone(int id) {
    for (auto &task : tasks) {
        if (task.getId() == id) {
            task.setStatus(TaskStatus::DONE);
            return true;
        }
    }
    return false;
}

void TaskManager::undoDelete() {
    if (!undoStack.isEmpty()) {
        tasks.append(undoStack.pop());
    }
}

QList<Task> TaskManager::getAllTasks() const {
    return tasks;
}

void TaskManager::loadFromApi(const QJsonArray &array)
{
    tasks.clear();
    undoStack.clear();

    for (const QJsonValue &value : array)
    {
        QJsonObject obj = value.toObject();

        int id = obj["task_id"].toInt();
        QString title = obj["title"].toString();
        QString description = obj["description"].toString();

        TaskStatus status;
        switch (obj["status"].toInt())
        {
        case 0:
            status = TaskStatus::TODO;
            break;
        case 1:
            status = TaskStatus::IN_PROGRESS;
            break;
        case 2:
            status = TaskStatus::DONE;
            break;
        default:
            status = TaskStatus::TODO;
        }

        int priority = obj["priority"].toInt();

        QDateTime deadline =
            QDateTime::fromString(obj["deadline"].toString(), Qt::ISODate);

        tasks.append(Task(id, title, description, status, priority, deadline));
    }
}

QList<Task> TaskManager::searchTasks(const QString &keyword) const {
    QList<Task> result;
    for (const auto &task : tasks) {
        if (task.getTitle().contains(keyword, Qt::CaseInsensitive)) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::filterByStatus(TaskStatus status) const {
    QList<Task> result;
    for (const auto &task : tasks) {
        if (task.getStatus() == status) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::filterByPriority(int priority) const {
    QList<Task> result;
    for (const auto &task : tasks) {
        if (task.getPriority() == priority) {
            result.append(task);
        }
    }
    return result;
}

void TaskManager::sortByDeadline() {
    std::sort(tasks.begin(), tasks.end(), [](const Task &a, const Task &b) {
        return a.getDeadline() < b.getDeadline();
    });
}

QList<Task> TaskManager::getOverdueTasks() const {
    QList<Task> result;
    for (const auto &task : tasks) {
        if (task.isOverdue()) {
            result.append(task);
        }
    }
    return result;
}

QList<Task> TaskManager::getTasksDueSoon(int hours) const {
    QList<Task> result;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime future = now.addSecs(hours * 3600);
    for (const auto &task : tasks) {
        if (task.getStatus() != TaskStatus::DONE && task.getDeadline() > now && task.getDeadline() <= future) {
            result.append(task);
        }
    }
    return result;
}

double TaskManager::getCompletionRate() const {
    if (tasks.isEmpty()) return 0.0;
    int doneCount = 0;
    for (const auto &task : tasks) {
        if (task.getStatus() == TaskStatus::DONE) {
            doneCount++;
        }
    }
    return (double)doneCount / tasks.size() * 100.0;
}

bool TaskManager::saveToFile(const QString &filePath) const {
    QJsonArray jsonArray;
    for (const auto &task : tasks) {
        QJsonObject taskObj;
        taskObj["id"] = task.getId();
        taskObj["title"] = task.getTitle();
        taskObj["description"] = task.getDescription();
        taskObj["status"] = Task::statusToString(task.getStatus());
        taskObj["priority"] = task.getPriority();
        taskObj["deadline"] = task.getDeadline().toString(Qt::ISODate);
        jsonArray.append(taskObj);
    }
    
    QJsonObject rootObj;
    rootObj["tasks"] = jsonArray;

    QJsonDocument doc(rootObj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(doc.toJson());
    file.close();
    return true;
}

bool TaskManager::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }
    
    if (!doc.isObject()) return false;
    
    QJsonObject rootObj = doc.object();
    
    QJsonArray jsonArray = rootObj["tasks"].toArray();
    tasks.clear();
    undoStack.clear();
    
    for (const auto &val : std::as_const(jsonArray)) {
        QJsonObject taskObj = val.toObject();
        
        int id = 0;
        if (taskObj["id"].isDouble()) {
            id = taskObj["id"].toInt();
        } else if (taskObj["id"].isString()) {
            id = taskObj["id"].toString().toInt();
        }

        QString title = taskObj["title"].toString();
        QString description = taskObj["description"].toString();
        TaskStatus status = Task::stringToStatus(taskObj["status"].toString());
        int priority = taskObj["priority"].toInt();
        QDateTime deadline = QDateTime::fromString(taskObj["deadline"].toString(), Qt::ISODate);
        
        tasks.append(Task(id, title, description, status, priority, deadline));
    }
    
    return true;
}
